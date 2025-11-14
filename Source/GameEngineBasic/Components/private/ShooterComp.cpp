
#include "HomingMissileProjectile.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "NiagaraFunctionLibrary.h"
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
}

bool UShooterComp::TryFire()
{
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
	if (CurrentAmmo <= 0)
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
	CurrentAmmo--;

	// 쿨다운
	bIsReadyToFire = false;
	GetWorld()->GetTimerManager().SetTimer(
		FireRateTimerHandle,
		this,
		&UShooterComp::ResetFireReady,
		FireRate,
		false);
	// 총구 위치 계산
	USceneComponent* MuzzleComp = MyOwner->FindComponentByClass<USkeletalMeshComponent>();
	if (!MuzzleComp)
	{
		MuzzleComp = MyOwner->FindComponentByClass<UStaticMeshComponent>();
	}

	const FVector SpawnLoc = MuzzleComp
		? MuzzleComp->GetSocketLocation(MuzzleSocketName)
		: MyOwner->GetActorLocation();

	FRotator SpawnRot;

	if (!FireDirection.IsNearlyZero())
	{
		SpawnRot = FireDirection.Rotation();
	}
	else if (MuzzleComp)
	{
		SpawnRot = MuzzleComp->GetSocketRotation(MuzzleSocketName);
	}
	else
	{
		SpawnRot = MyOwner->GetActorRotation();
	}

	// Projectile 스폰
	FActorSpawnParameters Params;
	Params.Owner = MyOwner;
	Params.Instigator = MyOwner->GetInstigator();
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


	AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		SpawnLoc,
		SpawnRot,
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

	if (Projectile)
	{
		Projectile->DamageAmount = PendingDamage;
		Projectile->SetActorScale3D(FVector(PendingScale));

		if (Projectile->GetProjectileMovement())
		{
			const FVector Direction = FireDirection.IsNearlyZero()
				? SpawnRot.Vector()
				: FireDirection;

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
			SpawnLoc,
			SpawnRot);
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