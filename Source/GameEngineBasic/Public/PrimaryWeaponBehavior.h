// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBehavior.h"
#include "PrimaryWeaponBehavior.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EPrimaryFireMode : uint8
{
	Semi UMETA(DisplayName="Semi"),
	Auto UMETA(DisplayName="Auto")
};

UCLASS()
class GAMEENGINEBASIC_API UPrimaryWeaponBehavior : public UWeaponBehavior
{
	GENERATED_BODY()

public:

	// FireMode: DT에서 읽어와서 세팅
	UPROPERTY()
	EPrimaryFireMode FireMode = EPrimaryFireMode::Semi;

	// 자동사격용 타이머
	FTimerHandle AutoFireTimerHandle;

	// 자동 사격 시, TryFire를 얼마나 자주 호출할지 (쿨타임이 아니라 “폴링 주기”)
	UPROPERTY(EditAnywhere, Category="Primary")
	float AutoFireTickInterval = 0.02f; // 50fps 정도로 충분
	
	// 초기화 시 WeaponData에서 모드 가져오기
	virtual void Initialize(UWeaponComponent* InWeapon) override;

	virtual void OnFirePressed_Implementation() override;
	virtual void OnFireReleased_Implementation() override;

protected:
	// 한 발 쏘는 실제 로직
	void FireOnce();

	// 자동사격 시작/중지
	void StartAutoFire();
	void StopAutoFire();
};