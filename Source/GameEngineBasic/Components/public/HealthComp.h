// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "HealthComp.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, AActor*, Owner, float, NewHealth, float, NewShield);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Owner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldBroken, AActor*, Owner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTaken, AActor*, Owner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged_Ver2, float, NewHealth, float, MaxHealth);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UHealthComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComp();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ���� ���� ��� �ð�(���� �ð�, ��). �� �ð��� ������ +1
	float NextRegenTime = -FLT_MAX;

public:	
	// ======== CONFIG ========
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Health")
	int32 MaxHealth = 60;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Shield")
	int32 MaxShield = 5;

	// �ǰ� �� ȸ�� ���۱��� ���
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield")
	float ShieldRegenDelay = 3.f;   

	UFUNCTION(BlueprintCallable, Category = "Health")
	int GetCurrentHealth() const { return CurrentHealth; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	int GetMaxHealth() const { return MaxHealth; }
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	int GetShield() const { return CurrentShield; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Health")
	bool bUseShield = true;   // 쉴드 사용 여부

	UPROPERTY(EditAnywhere, Category="Shield")
	bool bUseShieldRegen = true;

	UPROPERTY(EditAnywhere, Category="Invincible")
	bool bUseInvincibleFrame = true;

	UPROPERTY(EditAnywhere, Category="Invincible", meta=(EditCondition="bUseInvincibleFrame"))
	float InvincibleDuration = 0.3f;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugHealthLog = false;

	// ========== STATE ==========
	UPROPERTY(EditAnywhere, Category = "Stat")
	float CurrentHealth = 60;
	UPROPERTY(EditAnywhere, Category = "Stat")
	float CurrentShield = 4;
	float LastDamageTime = -FLT_MAX;

	FTimerHandle ShieldRegenTimerHandle;

	// ========== FUNCTIONS ==========
	void InitStats();
	void StartShieldRegenTimer();
	void StopShieldRegenTimer();

	bool IsInvincible() const;

public:
	// ========== EXTERNAL API ==========
	UFUNCTION(BlueprintCallable)
	void TakeDamage(int32 DamageAmount = 1);

	UFUNCTION(BlueprintCallable)
	void RestoreShield(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void ApplyShieldDamage(int Amount);

	UFUNCTION(BlueprintCallable)
	void ApplyHealthDamage(float Amount);

	// ========== DELEGATES ==========
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnHealthChanged_Ver2 OnHealthChanged_Ver2;
	
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnDeath OnDeath;

	UPROPERTY(BlueprintAssignable)
	FOnShieldBroken OnShieldBroken;

	UPROPERTY(BlueprintAssignable)
	FOnDamageTaken OnDamageTaken;

private:
	void BroadcastChanged() const;
};
