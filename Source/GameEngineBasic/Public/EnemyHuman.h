// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameFramework/Character.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "EnemyHuman.generated.h"

UCLASS()
class GAMEENGINEBASIC_API AEnemyHuman : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyHuman();

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void EntryGroggyState(FName Name);
	void OnDie();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void StartBoost(FVector Direction, float Speed, float Duration, float Decel, float GravityScale);
	void EndBoost();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UPawnSensingComponent* PawnSensingComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UShooterComp*  ShooterComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHealthComp* HealthComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	float AlertHoldSeconds = 3.0f; // 마지막으로 본 뒤 이 시간 동안 Alert 유지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	UBehaviorTree* BehaviorTreeAsset;
	bool        bIsBoosting = false;
	float       BoostTickInterval = 0.016f;
	float       BoostElapsed = 0.f;
	float       BoostDurationCached = 0.f;
	float       BoostSpeedCached = 0.f;
	float       GlideDecelRateCached = 0.f;
	float       OriginalGravityScale = 1.f;
	FVector     BoostDirCached = FVector::ZeroVector;

	FTimerHandle TimerHandle_BoostTick;

	void OnBoostTick(); // ← 타이머 콜백

protected:
	float CurrentHealth;
	float MaxHealth;
	TMap<FName,float> BodyPartDamage;
	TMap<FName, float> GroggyThreshold;

	UPROPERTY(EditAnywhere, Category = "Vfx")
	UNiagaraSystem* BoostVfx;

	UPROPERTY(EditAnywhere, Category = "Vfx")
	USoundBase* BoostSound;
private:
	FTimerHandle TimerHandle_BoostEnd;
};
