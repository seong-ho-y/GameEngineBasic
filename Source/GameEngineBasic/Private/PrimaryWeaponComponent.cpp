#include "PrimaryWeaponComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"

void UPrimaryWeaponComponent::HandleFirePressed()
{
	if (!CanFire()) return;

	if (bAutomatic)
	{
		// 즉시 한 발
		PerformFire();

		// 이후부터 일정 간격으로 연사
		GetWorld()->GetTimerManager().SetTimer(
			AutoFireHandle,
			this,
			&UPrimaryWeaponComponent::AutoFire,
			FireInterval,
			true
		);
	}
	else
	{
		// 기본 무기는 단발로 처리
		PerformFire();
	}
}

void UPrimaryWeaponComponent::HandleFireReleased()
{
	if (bAutomatic)
	{
		GetWorld()->GetTimerManager().ClearTimer(AutoFireHandle);
	}
}

void UPrimaryWeaponComponent::AutoFire()
{
	PerformFire();
}
