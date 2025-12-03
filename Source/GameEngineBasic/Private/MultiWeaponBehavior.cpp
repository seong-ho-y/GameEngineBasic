#include "MultiWeaponBehavior.h"
#include "WeaponComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/World.h"

void UMultiWeaponBehavior::OnFirePressed_Implementation()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	// DataTable 설정 가져오기
	SpreadCount = OwnerWeapon->WeaponData.SpreadCount;
	SpreadAngle = OwnerWeapon->WeaponData.SpreadAngle;

	// Primary와 동일: 즉시 한 번 발사
	FireOnce();
}

void UMultiWeaponBehavior::OnFireReleased_Implementation()
{
	// Multi는 AutoFire 없음 (Shotgun 용)
}

void UMultiWeaponBehavior::FireOnce()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	// 💥 Primary와 동일한 흐름: AimDirection → TryFire()
	FVector AimDir = OwnerWeapon->GetAimDirection();
	OwnerWeapon->ShooterComp->SetFireDirection(AimDir);

	bool bDidFire = OwnerWeapon->ShooterComp->TryFire();
	if (!bDidFire)
		return;

	// ShooterComp::Fire()가 1발 스폰했음
	// 그 뒤에 우리가 산탄 추가 스폰
	SpawnExtraProjectiles();
}

void UMultiWeaponBehavior::SpawnExtraProjectiles()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	UWorld* World = OwnerWeapon->GetWorld();
	if (!World) return;

	FVector BaseDir = OwnerWeapon->GetAimDirection();
	FVector MuzzleLoc = OwnerWeapon->GetMuzzleLoc();

	for (int32 i = 0; i < SpreadCount; i++)
	{
		FVector ShotDir = GetSpreadDirection(BaseDir);

		AProjectile* Proj = World->SpawnActor<AProjectile>(
			OwnerWeapon->WeaponData.ProjectileClass,
			MuzzleLoc,
			ShotDir.Rotation()
		);

		if (Proj && Proj->GetProjectileMovement())
		{
			// ShooterComp가 설정한 PendingDamage 사용!
			Proj->DamageAmount = OwnerWeapon->ShooterComp->PendingDamage;

			Proj->GetProjectileMovement()->Velocity =
				ShotDir * Proj->GetProjectileMovement()->InitialSpeed;
		}
	}
}

FVector UMultiWeaponBehavior::GetSpreadDirection(const FVector& BaseDir) const
{
	const float YawOffset = FMath::FRandRange(-SpreadAngle, SpreadAngle);
	const float PitchOffset = FMath::FRandRange(-SpreadAngle, SpreadAngle);

	FRotator Rot = BaseDir.Rotation();
	Rot.Yaw += YawOffset;
	Rot.Pitch += PitchOffset;

	return Rot.Vector();
}
