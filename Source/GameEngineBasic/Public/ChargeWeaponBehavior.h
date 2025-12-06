#pragma once

#include "CoreMinimal.h"
#include "WeaponBehavior.h"
#include "ChargeWeaponBehavior.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UChargeWeaponBehavior : public UWeaponBehavior
{
	GENERATED_BODY()

public:

	// ========== 내부 상태 ==========
	UPROPERTY()
	bool bIsCharging = false;

	UPROPERTY()
	float ChargeStartTime = 0.f;


	UPROPERTY()
	UParticleSystemComponent* ChargeVFXComp = nullptr;

	UPROPERTY()
	float ChargeHoldTime = 0.f;

	FTimerHandle ChargeTimerHandle;
	FTimerHandle OverChargeTimerHandle;

	// ========== Behavior API ==========
	virtual void OnFirePressed_Implementation() override;
	virtual void OnFireReleased_Implementation() override;

	// ========== 기존 Charge 로직 ==========
	void StartCharge();
	void UpdateCharge();
	void ReleaseCharge();
	void PerformChargedFire();
	float GetChargeRatio() const;

	// ========== 오버차지 ==========
	void HandleOverCharge();
	void OverchargeExplode();
};
