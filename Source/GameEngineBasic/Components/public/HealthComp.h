// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComp.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Owner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged_Ver2, float, NewHealth, float, MaxHealth);

class UShieldComp;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UHealthComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComp();

private:
	UPROPERTY()
	UShieldComp* ShieldComp;
protected:
	virtual void BeginPlay() override;

public:	
	// ======== CONFIG ========
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float CurrentHealth = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float InvincibleDuration = 1.f;

	UPROPERTY(EditAnywhere, Category="Debug")
	bool bDebugHealthLog = false;

	// ======== SOUND ========
	UPROPERTY(EditAnywhere, Category = "Sound|Hit")
	TArray<USoundBase*> HitSounds;

	// ========== FUNCTIONS ==========
	void InitStats();
	void RestoreFullHealth();

	// ========== GETTERS ==========
	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }
	bool GetIsInvincible() const { return bIsInvincible; }

public:
	// ========== EXTERNAL API ==========
	UFUNCTION(BlueprintCallable)
	float ApplyHealthDamage(float Damage);

	UFUNCTION(BlueprintCallable)
	void Heal(float Amount);

	//  Events
	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnHealthChanged_Ver2 OnHealthChanged_Ver2;

	UPROPERTY(BlueprintAssignable, Category = "HealthEvent")
	FOnDeath OnDeath;

private:
	bool bIsInvincible = false;
	FTimerHandle InvincibleTimerHandle;

	void StartInvincibility();
	void EndInvincibility();

	void BroadcastStatus();
	void HandleDeath();
};
