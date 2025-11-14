// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TargetingSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "GeometryCollection/GeometryCollectionConvexUtility.h"
#include "MyPlayerHUD.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UMyPlayerHUD : public UUserWidget
{
	GENERATED_BODY()


public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;

protected:
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* EnemyHealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* AmmoBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* FuelBar;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* CentralCircleImage;
	
private:
	UPROPERTY()
	UTargetingSystemComponent* CachedTarget;

	UTargetingSystemComponent* GetTargetingComp();
	UFUNCTION()
	void HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo);
	UFUNCTION()
	void HandleEnergyChanged(float CurrentEN, float MaxEN);

	UPROPERTY()
	APawn* OwningPawn;
	
	UPROPERTY()
	UTargetingSystemComponent* TargetingComp;

	UPROPERTY()
	UHealthComp* BoundEnemyHealthComp;

	// 보간용 값
	float TargetEnemyHPRatio = 0.f;
	float DisplayEnemyHPRatio = 0.f;

	UFUNCTION()
	void HandleTargetChanged(AEnemyHuman* NewTarget);

	UFUNCTION()
	void HandleEnemyHealthChanged(float NewHealth, float MaxHealth);

	void CacheReferences();
};
