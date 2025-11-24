#pragma once

#include "CoreMinimal.h"
#include "WeaponComponent.h"
#include "ChargeWeaponComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEENGINEBASIC_API UChargeWeaponComponent : public UWeaponComponent
{
	GENERATED_BODY()

public:
	UChargeWeaponComponent();

	virtual void HandleFirePressed() override;
	virtual void HandleFireReleased() override;

protected:
	virtual void BeginPlay() override;

	void StartCharge();
	void ReleaseCharge();
	void OverchargeExplode();

	float CurrentChargeTime = 0.f;
	FTimerHandle ChargeTickHandle;

	bool bIsCharging = false;
	
	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* OverchargeVFX;
};
