// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcGaugeWidget.h"
#include "TargetingSystemComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "GeometryCollection/GeometryCollectionConvexUtility.h"
#include "MyPlayerHUD.generated.h"

class UTextBlock;
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
	UFUNCTION()
	virtual void NativeConstruct() override;

protected:
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* EnemyHealthGauge;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* EnemyStunGauge;

	UPROPERTY(meta = (BindWidget))
	UArcGaugeWidget* AmmoGauge;
	
	UPROPERTY(meta = (BindWidget))
	UArcGaugeWidget* EnergyGauge;

	UPROPERTY(meta = (BindWidgetOptional))
	UImage* CentralCircleImage;


	
	UPROPERTY()
	UTargetingSystemComponent* TargetingComp;
	
	UPROPERTY()
	UTargetingSystemComponent* CachedTarget;

	UPROPERTY()
	APawn* OwningPawn;
	
	UPROPERTY()
	UHealthComp* BoundEnemyHealthComp;

	UPROPERTY()
	UEnemyShieldComponent* BoundEnemyShieldComp;

	float TargeRatio = 0.f;
	float DisplayCurrentRatio = 0.f;
	float CurrentStunRatio = 0.f;
	float DisplayAmmoRatio = 1.f;
	float TargetAmmoRatio = 1.f;
	float DisplayEnergyRatio = 1.f;
	float TargetEnergyRatio = 1.f;


	void CacheReferences();

	UFUNCTION()
	void HandleTargetChanged(AEnemyHuman* NewTarget);

	UFUNCTION()
	void HandleEnemyHealthChanged(float NewHealth, float MaxHealth);

	UFUNCTION()
	void HandleEnemyShieldDamaged();

	UFUNCTION()
	void HandleEnemyShieldRestored();

	UFUNCTION()
	void HandleAmmoChanged(int32 CurrentAmmo, int32 FullAmmo, int32 InMaxAmmo);

	UFUNCTION()
	void HandleEnergyChanged(float CurrentEN, float MaxEN);

public:
	UTargetingSystemComponent* GetTargetingComp();


	// 보유한 총 탄약 개수
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxAmmo;


private:
	float EnemyUIFadeTime = 0.15f;
	float EnemyUIOpacity = 0.f;
	float TargetEnemyUIOpacity = 0.f;
	float EnemyUIFadeSpeed = 8.f;
};
