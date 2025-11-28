#include "MultiWeaponBehavior.h"
#include "WeaponComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile.h"

void UMultiWeaponBehavior::OnFirePressed_Implementation()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	// DataTable 설정 로드
	SpreadCount = OwnerWeapon->WeaponData.SpreadCount;
	SpreadAngle = OwnerWeapon->WeaponData.SpreadAngle;

	// ───────────────────────────────
	// 1) ShooterComp로 "1발만" TryFire 호출
	//    (쿨타임 / 탄약 / Reload 정상 작동)
	// ───────────────────────────────
	bool bDidFire = OwnerWeapon->ShooterComp->TryFire();
	if (!bDidFire) return;

	// ShooterComp가 Spawn한 Projectile은 Player Shotgun에서는 사용하지 않음
	// → 기본 1발 스폰을 제거하려면 ShooterComp.Fire() 소스 수정이 필요하지만
	//   "Player만" 사용이므로 기본 Projectile은 그냥 손대지 않음.
	//   대신 Multi-Spawn의 위력을 더 크게 한다 (샷건 특성)

	// ───────────────────────────────
	// 2) Behavior가 SpreadCount 만큼 직접 생성
	// ───────────────────────────────
	SpawnMultiProjectiles();
}

void UMultiWeaponBehavior::SpawnMultiProjectiles()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	UWorld* World = OwnerWeapon->GetWorld();
	if (!World) return;

	FVector BaseDir = OwnerWeapon->GetAimDirection();
	FVector MuzzleLoc = OwnerWeapon->GetMuzzleLoc();

	for (int32 i = 0; i < SpreadCount; i++)
	{
		FVector ShotDir = MakeSpreadDirection(BaseDir);

		AProjectile* Proj = World->SpawnActor<AProjectile>(
			OwnerWeapon->WeaponData.ProjectileClass,
			MuzzleLoc,          // 위치
			ShotDir.Rotation()  // 회전
		);

		if (Proj && Proj->GetProjectileMovement())
		{
			Proj->DamageAmount = OwnerWeapon->WeaponData.Damage;

			Proj->GetProjectileMovement()->Velocity =
				ShotDir * Proj->GetProjectileMovement()->InitialSpeed;
		}
	}
}

FVector UMultiWeaponBehavior::MakeSpreadDirection(const FVector& BaseDir) const
{
	const float YawOffset = FMath::FRandRange(-SpreadAngle, SpreadAngle);
	const float PitchOffset = FMath::FRandRange(-SpreadAngle, SpreadAngle);

	FRotator NewRot = BaseDir.Rotation();
	NewRot.Yaw += YawOffset;
	NewRot.Pitch += PitchOffset;

	return NewRot.Vector();
}
