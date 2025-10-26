// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "HealthComp.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShieldBroken, AActor*, Owner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHealthChanged, AActor*, Owner, float, NewHealth, float, NewShield);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, Owner);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UHealthComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComp();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	int CurrentHealth;
	int CurrentShield;

	// 다음 쉴드 재생 시각(게임 시간, 초). 이 시간이 지나면 +1
	float NextRegenTime = -FLT_MAX;

public:
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnShieldBroken OnShieldBroken;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnDeath OnDeath;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Health")
	int MaxHealth = 1;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Shield")
	int MaxShield = 5;

	// 피격 후 회복 시작까지 대기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Shield")
	float ShieldRegenDelay = 3.f;    

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield")
	float LastDamageTime = -FLT_MAX;

public:
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitStats();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void RestoreShield(int Amount);

	UFUNCTION(BlueprintCallable, Category = "Health")
	int GetHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintCallable, Category = "Health")
	int GetShield() const { return CurrentShield; }
	

public:
	void ApplyShieldDamage(int  Amount); // 실드 감소, 0 도달 시 OnShieldBroken.Broadcast
	void ApplyHealthDamage(int  Amount); // 체력 감소, 0 도달 시 OnDeath.Broadcast
	void BroadcastChanged();
};
