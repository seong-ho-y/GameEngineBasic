#include "PrimaryWeaponBehavior.h"
#include "WeaponComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UPrimaryWeaponBehavior::Initialize(UWeaponComponent* InWeapon)
{
	Super::Initialize(InWeapon);
	
	if (!OwnerWeapon) return;

	// 여기서 WeaponData에서 FireMode 정보 읽기
	// 예시: FWeaponData 안에 bool bIsAuto 가 있다고 가정
	if (OwnerWeapon->WeaponData.bIsAuto)
	{
		FireMode = EPrimaryFireMode::Auto;
	}
	else
	{
		FireMode = EPrimaryFireMode::Semi;
	}
}

void UPrimaryWeaponBehavior::OnFirePressed_Implementation()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	switch (FireMode)
	{
	case EPrimaryFireMode::Semi:
		FireOnce(); // 한 번만
		break;

	case EPrimaryFireMode::Auto:
		StartAutoFire(); // 타이머 돌려서 연사
		break;
	}
}

void UPrimaryWeaponBehavior::OnFireReleased_Implementation()
{
	if (FireMode == EPrimaryFireMode::Auto)
	{
		StopAutoFire();
	}
}

void UPrimaryWeaponBehavior::FireOnce()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	FVector AimDir = OwnerWeapon->GetAimDirection();
	OwnerWeapon->ShooterComp->SetFireDirection(AimDir);
	OwnerWeapon->ShooterComp->TryFire();
}

void UPrimaryWeaponBehavior::StartAutoFire()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;
	if (UWorld* World = OwnerWeapon->GetWorld())
	{
		// 눌렀을 때 즉시 한 번 쏘고
		FireOnce();

		// 이후에는 짧은 간격으로 TryFire 반복 호출
		World->GetTimerManager().SetTimer(
			AutoFireTimerHandle,
			this,
			&UPrimaryWeaponBehavior::FireOnce,
			AutoFireTickInterval,
			true
		);
	}
}

void UPrimaryWeaponBehavior::StopAutoFire()
{
	if (!OwnerWeapon) return;

	if (UWorld* World = OwnerWeapon ? OwnerWeapon->GetWorld() : nullptr)
	{
		World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
	}
}
