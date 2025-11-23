// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponComponent.h"
#include "PrimaryWeaponComponent.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UPrimaryWeaponComponent : public UWeaponComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	bool bAutomatic = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireInterval = 0.1f;

public:
	virtual void HandleFirePressed() override;
	virtual void HandleFireReleased() override;

protected:
	FTimerHandle AutoFireHandle;
	void AutoFire();
};
