// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShieldComp.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldDeactivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldBroken);

class USphereComponent;
class AShieldActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UShieldComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UShieldComp();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	bool bShieldActive = false;

	UPROPERTY()
	bool bCanShield = true;
public:
	// ===== 쉴드 스탯 =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield")
	float MaxShield = 50.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield")
	float CurrentShield = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield")
	float ShieldDuration = 20.f;

	UPROPERTY(EditAnywhere, Category = "Shield|Effects")
	UParticleSystem* HitEffect;

	UPROPERTY(EditAnywhere, Category = "Shield|Effects")
	UParticleSystem* ShieldBroken;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Shield")
	USphereComponent* ShieldCollision;

public:
	// ===== 기능 함수 =====
	UFUNCTION(BlueprintCallable)
	void ActivateShield();

	UFUNCTION(BlueprintCallable)
	void DeactivateShield();

	UFUNCTION(BlueprintCallable)
	bool IsShieldActive() const { return bShieldActive; }

	float ApplyShieldDamage(float Damage);

	

private:
	UFUNCTION()
	void OnShieldHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	FTimerHandle ShieldCoolDownTimer;
	void StartShieldCoolDown();
	void EndShieldCoolDown();

public:
	// 이벤트
	UPROPERTY(BlueprintAssignable)
	FOnShieldActivated OnShieldActivated;

	UPROPERTY(BlueprintAssignable)
	FOnShieldDeactivated OnShieldDeactivated;

	UPROPERTY(BlueprintAssignable)
	FOnShieldBroken OnShieldBroken;
};

