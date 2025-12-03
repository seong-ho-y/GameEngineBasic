#include "ChargeWeaponBehavior.h"
#include "WeaponComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "SpaceCharacter/SpaceCharacter.h"

void UChargeWeaponBehavior::OnFirePressed_Implementation()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;
	StartCharge();
}

void UChargeWeaponBehavior::OnFireReleased_Implementation()
{
	if (!bIsCharging) return;
	ReleaseCharge();
}

void UChargeWeaponBehavior::StartCharge()
{
	if (!OwnerWeapon) return;

	bIsCharging = true;
	ChargeStartTime = OwnerWeapon->GetWorld()->GetTimeSeconds();

	// === 최소 차지 시간 / 차지 누적 타이머 ===
	OwnerWeapon->GetWorld()->GetTimerManager().SetTimer(
		ChargeTimerHandle,
		this,
		&UChargeWeaponBehavior::UpdateCharge,
		0.01f,
		true
	);

	// === 오버차지 타이머 ===
	if (OwnerWeapon->WeaponData.bIsChargeWeapon &&
		OwnerWeapon->WeaponData.OverchargeTime > 0.f)
	{
		OwnerWeapon->GetWorld()->GetTimerManager().SetTimer(
			OverChargeTimerHandle,
			this,
			&UChargeWeaponBehavior::HandleOverCharge,
			OwnerWeapon->WeaponData.OverchargeTime,
			false
		);
	}
}

void UChargeWeaponBehavior::UpdateCharge()
{
	if (!OwnerWeapon) return;

	const float Now = OwnerWeapon->GetWorld()->GetTimeSeconds();
	ChargeHoldTime = Now - ChargeStartTime;
}

void UChargeWeaponBehavior::ReleaseCharge()
{
	if (!OwnerWeapon) return;

	bIsCharging = false;

	OwnerWeapon->GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
	OwnerWeapon->GetWorld()->GetTimerManager().ClearTimer(OverChargeTimerHandle);

	PerformChargedFire();
}

void UChargeWeaponBehavior::PerformChargedFire()
{
	if (!OwnerWeapon || !OwnerWeapon->ShooterComp) return;

	float Ratio = GetChargeRatio();
	float FinalDamage = OwnerWeapon->WeaponData.Damage *
		FMath::Lerp(1.f, 10.f, Ratio);

	OwnerWeapon->ShooterComp->PendingDamage = FinalDamage;
	OwnerWeapon->ShooterComp->ProjectileClass =
		OwnerWeapon->WeaponData.ProjectileClass;

	// 기존 코드 그대로
	FVector AimDir = OwnerWeapon->GetAimDirection();
	OwnerWeapon->ShooterComp->SetFireDirection(AimDir);
	OwnerWeapon->ShooterComp->TryFire();
}

float UChargeWeaponBehavior::GetChargeRatio() const
{
	if (!OwnerWeapon) return 0.f;
	return FMath::Clamp(
		ChargeHoldTime / OwnerWeapon->WeaponData.ChargeTime,
		0.f, 1.f
	);
}

void UChargeWeaponBehavior::HandleOverCharge()
{
	bIsCharging = false;

	OwnerWeapon->GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OverCharged!!"));

	OverchargeExplode();
}

void UChargeWeaponBehavior::OverchargeExplode()
{
	if (!OwnerWeapon) return;
	ASpaceCharacter* OwnerChar = OwnerWeapon->OwnerCharacter;

	// 데미지 적용
	if (OwnerChar)
	{
		UGameplayStatics::ApplyDamage(
			OwnerChar, 50.f, nullptr, nullptr, nullptr
		);
	}

	// 폭발 이펙트
	if (OwnerWeapon->WeaponData.OverChargeVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			OwnerWeapon->GetWorld(),
			OwnerWeapon->WeaponData.OverChargeVFX,
			OwnerChar->GetActorLocation()
		);
	}
}
