// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "AnimNodes/AnimNode_RandomPlayer.h"
#include "Components/ActorComponent.h"
#include "EnemyShieldComponent.generated.h"
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldBreak);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldRestored);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldDamaged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UEnemyShieldComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEnemyShieldComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	float ApplyDamage(float Damage);
	void RegenerateShield(float DeltaTime);
	void RestoreShieldFull();
	void ShieldBrokenVFX();
	void ShieldResotreVFX();
	
	bool IsShieldBroken() const {return bIsShieldBroken;}
	bool CanBeExecuted() const {return bCanExecuted;}
	float GetShieldRatio() const {return CurrentShield/MaxShield;}
	float GetCurrentShield() const {return CurrentShield;}
	float GetMaxShield() const {return MaxShield;}

	/** 쉴드 이벤트 */
	UPROPERTY(BlueprintAssignable, Category="Shield|Event")
	FOnShieldBreak OnShieldBreak;
	UPROPERTY(BlueprintAssignable, Category="Shield|Event")
	FOnShieldRestored OnShieldRestored;
	UPROPERTY(BlueprintAssignable, Category="Shield|Event")
	FOnShieldDamaged OnShieldDamaged;

protected:
	/** ===== 기본 스탯 ===== */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Stat")
	float MaxShield = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Stat")
	float CurrentShield = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Stat")
	float RecoverSpeed = 6.f; // 초당 회복량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Stat")
	float RecoverDelay = 3.f; // 피격 후 회복 시작까지 딜레이

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Stat")
	float DamageReduction = 0.7f; // 데미지 경감률

	/** ===== Execution 관련 ===== */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Shield|Execution")
	float ExecutionTime = 8.f; // 처형 가능 유지시간

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|State")
	bool bIsShieldBroken = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Shield|State")
	bool bCanExecuted = false;

	UPROPERTY(EditAnywhere, Category = "Shield|VFX")
	USoundBase* ShieldBrokenSound;
	
	UPROPERTY(EditAnywhere, Category = "Shield|VFX")
	UNiagaraSystem* ShieldBrokenVfx;
	
	UPROPERTY(EditAnywhere, Category = "Shield|VFX")
	USoundBase* ShieldRestoreSound;
	
	UPROPERTY(EditAnywhere, Category = "Shield|VFX")
	UNiagaraSystem* ShieldResotreVfx;
	

	/** 내부 관리용 */
	FTimerHandle TimerHandle_ExecutionReset;
	float TimeSinceLastDamage = 0.f;
		
};
