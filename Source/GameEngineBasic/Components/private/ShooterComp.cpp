#include "../Components/public/ShooterComp.h"
#include "HomingMissileProjectile.h"
#include "MyPlayerHUD.h"
#include "NiagaraFunctionLibrary.h"
#include "WeaponComponent.h"
#include "AnimNodes/AnimNode_RandomPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

UShooterComp::UShooterComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UShooterComp::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemp, Warning, TEXT("[%s] BeginPlay CurrentAmmo = %d"), *GetOwner()->GetName(), CurrentAmmo);
	/*UE_LOG(LogTemp, Warning, TEXT("[%s] ShooterComp BeginPlay: bUseArcHoming=%d"),
		*GetOwner()->GetName(), bUseArcHoming);
	 */
}


void UShooterComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsReloading)
		return;

	// Reload 진행 시간 감소
	ReloadTimeRemaining -= DeltaTime;
	ReloadTimeRemaining = FMath::Max(0.f, ReloadTimeRemaining);

	float Elapsed = ReloadTimeTotal - ReloadTimeRemaining;
	float ReloadPercent = Elapsed / ReloadTimeTotal;
	ReloadPercent = FMath::Clamp(ReloadPercent, 0.f, 1.f);

	// 새로 계산된 Ammo 값
	int32 NewAmmo = FMath::RoundToInt(ReloadPercent * FullAmmo);

	// 현재 가지고 있는 최대로 채울 수 있는 ammo를 넘지 않도록
	int32 MaxCanFill = FMath::Min(FullAmmo, MaxAmmo);

	NewAmmo = FMath::Clamp(NewAmmo, 0, MaxCanFill);

	// HUD 업데이트
	if (NewAmmo != CurrentAmmo)
	{
		CurrentAmmo = NewAmmo;
		OnAmmoChanged.Broadcast(CurrentAmmo, FullAmmo, MaxAmmo);
	}

	// Reload 종료 조건 1 : 시간이 다 됨
	if (ReloadTimeRemaining <= 0.f)
	{
		ReloadSuccess();
		return;
	}

	// Reload 종료 조건 2 : 더 이상 채울 Ammo가 없음
	if (CurrentAmmo >= MaxCanFill)
	{
		ReloadSuccess();
		return;
	}
}

void UShooterComp::Fire_Implementation()
{
	GEngine->AddOnScreenDebugMessage(5843, 3.f, FColor::Red, TEXT("ShooterComp Fire Start"));
	
	AActor* MyOwner = GetOwner();
	if (!MyOwner)
	{
		return;
	}

	SetProjectile();

	if (!ProjectileClass)
	{
		//UE_LOG(LogTemp, Warning, TEXT("ShooterComp: No ProjectileClass found for type %d"), (int32)CurrentProjectileType);
		return;
	}
	// 탄약감소
	if (bUseAmmo)
	{
		CurrentAmmo--;
		OnAmmoChanged.Broadcast(CurrentAmmo, FullAmmo, MaxAmmo);
	}
	
	// 쿨다운
	bIsReadyToFire = false;
	GetWorld()->GetTimerManager().SetTimer(
		FireRateTimerHandle,
		this,
		&UShooterComp::ResetFireReady,
		FireRate,
		false);
	// 총구 위치 계산
	// 1. WeaponComp에서 할당된 Muzzle 있는지 확인
	// 2. Owner의 SkeletalMesh or StaticMesh 접근해서 Muzzle 찾기
	// 3. 없으면 Owner->GetActorLocation()
	FVector SpawnLoc = bHasExternalMuzzleInfo ? ExternalMuzzleLoc : FindMuzzleLoc();
	FRotator DummyRot;
	// Projectile 스폰 파라미터 설정
	FActorSpawnParameters Params;
	Params.Owner = MyOwner;
	Params.Instigator = MyOwner->GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		SpawnLoc,
		DummyRot,
		Params);
	// =========== 임시 로그 ===========
	if (!Projectile)
	{
		//UE_LOG(LogTemp, Error, TEXT("Fire : SpawnActor FAILED: %s"), *ProjectileClass->GetName());
		return;
	}
	/*UE_LOG(LogTemp, Warning, TEXT("Fire: Projectile Spawned: %s at %s"),
		   *Projectile->GetName(),
		   *SpawnLoc.ToString());
	*/

	// 프로젝타일 스폰 성공 시 데미지 & 방향 설정
	if (Projectile)
	{
		Projectile->DamageAmount = PendingDamage;
		Projectile->SetActorScale3D(FVector(PendingScale));

		if (Projectile->GetProjectileMovement())
		{
			const FVector Direction = FireDirection.IsNearlyZero()
				? FVector::ZeroVector
				: FireDirection;


			// 프로젝타일 방향 계산 후 설정
			Projectile->GetProjectileMovement()->Velocity =
				Direction * Projectile->GetProjectileMovement()->InitialSpeed;
		}
	}

	// 프로젝타일이 호밍미사일일 경우 (2가지 케이스 나누어서 전달)
	if (AHomingMissileProjectile* Missile = Cast<AHomingMissileProjectile>(Projectile))
	{
		//UE_LOG(LogTemp, Log, TEXT("Missile Projectile Mode Start"));
		Missile->SetHomingTarget(CurrentTarget);
		Missile->SetHomingType(bUseArcHoming ? EHomingType::ArcHoming : EHomingType::DirectHoming);
	}
	// 비주얼 / 사운드 효과
	if (MuzzleFlashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			MuzzleFlashEffect,
			SpawnLoc);
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, SpawnLoc);
	}
}


void UShooterComp::StartReload()
{
	if (bIsReloading)
		return;

	if (CurrentAmmo >= FullAmmo || CurrentAmmo >= MaxAmmo)
		return; // 이미 FULL 이면 필요 없음

	bIsReloading = true;

	ReloadTimeTotal = ReloadTime;
	ReloadTimeRemaining = ReloadTime;

	UE_LOG(LogTemp, Log, TEXT("Reload Start: ReloadTime = %.2f"), ReloadTime);
}


void UShooterComp::ReloadSuccess()
{
	bIsReloading = false;

	// MaxAmmo와 FullAmmo 비교
	int32 MaxCanFill = FMath::Min(FullAmmo, MaxAmmo);

	CurrentAmmo = MaxCanFill;
	MaxAmmo -= CurrentAmmo;
	
	OnAmmoChanged.Broadcast(CurrentAmmo, FullAmmo, MaxAmmo);

	UE_LOG(LogTemp, Log, TEXT("Reload Success → Ammo = %d / %d"), CurrentAmmo, FullAmmo);
}

bool UShooterComp::TryFire()
{
	GEngine->AddOnScreenDebugMessage(5843, 3.f, FColor::Red, TEXT("ShooterComp TryFire Start"));
	if (bIsReloading) return false;
	
	if (bUseAmmo && CurrentAmmo <= 0)
	{
		StartReload();
		return false;
	}
	if (!CanFire())
	{
		return false;
	}
	Fire();
	return true;
}

void UShooterComp::SetFireDirection(const FVector& NewDir)
{
	FireDirection = NewDir.GetSafeNormal();
}

bool UShooterComp::CanFire() const
{
	// 1. Check Ammo
	if (bUseAmmo && CurrentAmmo <= 0)
	{
		GEngine->AddOnScreenDebugMessage(5843, 3.f, FColor::Red, TEXT("NoAmmo"));
		return false;
	}
	// 2. Check Cooldown
	if (!bIsReadyToFire)
	{
		GEngine->AddOnScreenDebugMessage(5843, 3.f, FColor::Red, TEXT("NotReadyToFire"));
		return false;
	}

	if (!ProjectileClass && ProjectileMap.Num() == 0)
	{
		//UE_LOG(LogTemp, Error, TEXT("CanFire: No ProjectileClass and ProjectileMap is empty"));
		GEngine->AddOnScreenDebugMessage(5843, 3.f, FColor::Red, TEXT("NoProjectileClass"));
		return false;
	}
	//UE_LOG(LogTemp, Warning, TEXT("CanFire: PASSED"));

	return true;
}


void UShooterComp::ResetFireReady()
{
	bIsReadyToFire = true;
}

void UShooterComp::SetProjectile()
{
	// 플레이어 무기라면 무조건 WeaponComponent 우선
	if (GetOwner()->FindComponentByClass<UWeaponComponent>())
		return;
	
	// ---- AI/적용 fallback ----
	TSubclassOf<AProjectile>* FoundClass = ProjectileMap.Find(CurrentProjectileType);
	if (FoundClass && *FoundClass)
	{
		ProjectileClass = *FoundClass;
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("ShooterComp: No ProjectileClass found!!"));
}


void UShooterComp::SetMuzzle(const FVector& Loc)
{
	bHasExternalMuzzleInfo = true;
	ExternalMuzzleLoc = Loc;
}
void UShooterComp::ClearMuzzle()
{
	bHasExternalMuzzleInfo = false;
}

FVector UShooterComp::FindMuzzleLoc() const
{
	FVector SpawnLoc;
	
	// 1. 스켈레탈 메쉬를 먼저 확인
	USceneComponent* MuzzleComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	// 2. 스켈레탈 메쉬 없으면 스태틱 메쉬 확인
	if (!MuzzleComp)
		MuzzleComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();

	// 3. Mesh에서 MuzzleSocketName에 맞는 Muzzle 위치 가져오기
	if (MuzzleComp)
	{
		SpawnLoc = MuzzleComp->GetSocketLocation(MuzzleSocketName);
	}
	// 4. Muzzle이 없으면 그냥 액터의 위치 반환
	else
	{
		SpawnLoc = GetOwner()->GetActorLocation();
	}
	return SpawnLoc;
}