#include "../Components/public/ShooterComp.h"
#include "HomingMissileProjectile.h"
#include "NiagaraFunctionLibrary.h"
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


// Called every frame
void UShooterComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsReloading)
	{
		ReloadTimeRemaining -= DeltaTime;
		float ReloadPercent = 1.f - (ReloadTimeRemaining / ReloadTimeTotal);

		int32 TargetAmmo = FMath::FloorToInt(ReloadPercent * FullAmmo);
		TargetAmmo = FMath::Clamp(TargetAmmo, 0, FullAmmo);

		if (CurrentAmmo < TargetAmmo)
		{
			CurrentAmmo++;
			OnAmmoChanged.Broadcast(CurrentAmmo, FullAmmo);
		}
		if (ReloadTimeRemaining <= 0.f)
		{
			ReloadSuccess();
		}
	}
}

bool UShooterComp::TryFire()
{
	if (bIsReloading) return false;
	
	if (bUseAmmo && CurrentAmmo <= 0)
	{
		StartReload( 3.0f /* 임시 값, 나중에 무기별로 장전시간 변수 만들어서 넣어줄 예정 */ );
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
		return false;
	}
	// 2. Check Cooldown
	if (!bIsReadyToFire)
	{
		return false;
	}

	if (!ProjectileClass && ProjectileMap.Num() == 0)
	{
		//UE_LOG(LogTemp, Error, TEXT("CanFire: No ProjectileClass and ProjectileMap is empty"));
		return false;
	}
	//UE_LOG(LogTemp, Warning, TEXT("CanFire: PASSED"));

	return true;
}

void UShooterComp::Fire_Implementation()
{
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
		OnAmmoChanged.Broadcast(CurrentAmmo, FullAmmo);
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

void UShooterComp::ResetFireReady()
{
	bIsReadyToFire = true;
}

void UShooterComp::SetProjectile()
{
	TSubclassOf<AProjectile>* FoundClass = ProjectileMap.Find(CurrentProjectileType);
	if (FoundClass && *FoundClass)
	{
		ProjectileClass = *FoundClass;
		/*UE_LOG(LogTemp, Warning, TEXT("SetProjectile: Map[%d] -> %s"),
			(int32)CurrentProjectileType,
			*ProjectileClass->GetName());
		*/
	}
	// 맵에는 없지만 기존 ProjectileClass가 있으면 그대로 사용 (정상 상황)
	else if (ProjectileClass)
	{
		UE_LOG(LogTemp, Verbose, TEXT("SetProjectile: Using existing ProjectileClass -> %s"), *ProjectileClass->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetProjectile: No Projectile for Type %d AND no fallback ProjectileClass"), (int32)CurrentProjectileType);
	}
}

void UShooterComp::StartReload(float ReloadTime)
{
	if (MaxAmmo <= 0 || bIsReloading)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Fail to Reload"));
		return;
	}
	bIsReloading = true;
	ReloadTimeTotal = ReloadTime;
	ReloadTimeRemaining = ReloadTime;
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Emerald, TEXT("Reloading..."));
}
void UShooterComp::ReloadSuccess()
{
	CurrentAmmo = FMath::Min(FullAmmo, MaxAmmo);
	bIsReloading = false;
	OnAmmoChanged.Broadcast(CurrentAmmo, FullAmmo);
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
