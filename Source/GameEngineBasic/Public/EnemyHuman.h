// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyAnimInstance.h"
#include "EnemyBlade.h"
#include "EnemyShieldComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "GameFramework/Character.h"
#include "Particles/ParticleSystemComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "EnemyHuman.generated.h"

class UHealthComp;
class UShooterComp;

UCLASS()
class GAMEENGINEBASIC_API AEnemyHuman : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyHuman();

protected:
	UFUNCTION()
	void DisabledMovementAndAI();
	UFUNCTION()
	void OnExecutionStart(AActor* TargetEnemy);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
		
	void EntryGroggyState(FName Name);
	UFUNCTION()
	void OnDie(AActor* DeadActor);
	UFUNCTION(BlueprintCallable)
	void SetLowerBodyState(ELowerBodyState NewState);
	UFUNCTION(BlueprintCallable)
	void SetUpperBodyState(EUpperBodyState NewState);
	

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	void SetOutlineEnabled(bool bCond);
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void StartBoost(FVector Direction, float Speed, float Duration, float Decel, float GravityScale);
	void EndBoost();
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable)
	bool IsMeleeFinished() const;
	void StartMeleeAttack();
	void OnMeleeBegin();
	void OnMeleeEnd();
	void OnMeleeFinished();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UPawnSensingComponent* PawnSensingComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UShooterComp*  ShooterComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UHealthComp* HealthComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UEnemyShieldComponent* ShieldComp;
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
public:
	bool bIsKnocked;
	bool bIsDead;
	bool bIsExecuting;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AEnemyBlade> BladeBP;
	UPROPERTY()
	AEnemyBlade* Blade;

	void OnBoostTick(); // ← 타이머 콜백
	UFUNCTION()
	void OnKnock();

	
protected:
	float CurrentHealth;
	float MaxHealth;
	TMap<FName,float> BodyPartDamage;
	TMap<FName, float> GroggyThreshold;

	UPROPERTY(EditAnywhere, Category = "Vfx")
	UNiagaraSystem* BoostVfx;

	UPROPERTY(EditAnywhere, Category = "Vfx")
	USoundBase* BoostSound;

	UPROPERTY(EditAnywhere, Category = "VFX")
	UParticleSystem* BoostPS;

	UPROPERTY()
	UParticleSystemComponent* ActiveBoostPSC = nullptr;

	/** 근접 공격 데미지 */
	UPROPERTY(EditAnywhere, Category="Melee")
	float MeleeDamage = 25.f;

	/** 근접 공격의 범위 (캡슐, 구체, 반원 등) */
	UPROPERTY(EditAnywhere, Category="Melee")
	float MeleeRange = 120.f;

	/** 히트박스가 켜져있는 동안만 true */
	bool bMeleeHitboxActive = false;

	/** Task 종료 판단용 */
	bool bMeleeFinished = false;

	/** 어떤 본(Bone) 기준으로 반경 체크할지 */
	UPROPERTY(EditAnywhere, Category="Melee")
	FName MeleeHitSocket = "MeleeSocket";
	

	/** 이미 타격된 액터 중복 방지를 위한 캐시 */
	UPROPERTY()
	TSet<AActor*> MeleeAlreadyHitActors;
	
	// ==== Animation Montages ====
	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* BoostMontage;

	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* FireMontage;
	
	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* KnockMontage;
	
	UPROPERTY(EditAnywhere, Category = "Animation|Montage")
	UAnimMontage* ExecutionMontage;
	
	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* DeathMontage;

private:
	FTimerHandle TimerHandle_BoostEnd;
	bool bPulseActive = false;
	float PulseTime = 0.f;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicMIDs;
};