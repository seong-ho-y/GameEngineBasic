// EnemyHuman.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/PawnSensingComponent.h"
#include "EnemyBlade.h"
#include "EnemyShieldComponent.h"
#include "EnemyAnimInstance.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnemyHuman.generated.h"

class UHealthComp;
class UShooterComp;
class UNiagaraSystem;
class USoundBase;
class UAnimMontage;
class UParticleSystem;
class UParticleSystemComponent;

// DashAttack Task에서 쓸 Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBossDashFinishedDelegate);

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

	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	void OnLeftBladeBegin();
	void OnLeftBladeEnd();
	void PlayBladeAppearSFX();
	void PlayBladeSwingSFX();
	UPROPERTY(EditAnywhere, Category="SFX|Melee")
	USoundBase* BladeAppearSFX;

	UPROPERTY(EditAnywhere, Category="SFX|Melee")
	USoundBase* BladeSwingSFX;

	/* ===========================
	 *  상태 / 실행 / 넉백 관련
	 * ===========================
	 */
protected:
	void EntryGroggyState(FName Bone);

	UFUNCTION()
	virtual void OnDie(AActor* DeadActor);

	UFUNCTION()
	virtual void OnKnock();

	UFUNCTION()
	void OnExecutionStart(AActor* TargetEnemy);

	void DisabledMovementAndAI();

	UFUNCTION(BlueprintCallable)
	void SetLowerBodyState(ELowerBodyState NewState);

	UFUNCTION(BlueprintCallable)
	void SetUpperBodyState(EUpperBodyState NewState);

public:
	bool bIsKnocked = false;
	bool bIsDead = false;
	bool bIsExecuting = false;
	float ShortDashElapsed;

	/* ===========================
	 *  아웃라인 / 머티리얼
	 * ===========================
	 */
public:
	void SetOutlineEnabled(bool bCond);

protected:
	bool bPulseActive = false;
	float PulseTime = 0.f;

	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicMIDs;

	/* ===========================
	 *  부스트(Boost) 시스템
	 * ===========================
	 */
public:
	void StartBoost(FVector Direction, float Speed, float Duration, float Decel, float GravityScale);
	void EndBoost();

	UFUNCTION()
	void OnBoostTick();

protected:
	bool   bIsBoosting = false;
	float  BoostTickInterval = 0.016f;
	float  BoostElapsed = 0.f;
	float  BoostDurationCached = 0.f;
	float  BoostSpeedCached = 0.f;
	float  GlideDecelRateCached = 0.f;
	float  OriginalGravityScale = 1.f;
	FVector BoostDirCached = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category="Vfx")
	UNiagaraSystem* BoostVfx;
	
	UPROPERTY(EditAnywhere, Category="Vfx")
	USoundBase* BoostSound;

	UPROPERTY(EditAnywhere, Category="VFX")
	UParticleSystem* BoostPS;

	UPROPERTY()
	UParticleSystemComponent* ActiveBoostPSC = nullptr;

	FTimerHandle TimerHandle_BoostTick;
	FTimerHandle TimerHandle_BoostEnd;

	/* ===========================
	 *  근접(Melee) 시스템
	 * ===========================
	 */
public:
	void StartMeleeAttack();
	void OnMeleeBegin();
	void OnMeleeEnd();

	UFUNCTION(BlueprintCallable)
	bool IsMeleeFinished() const;

	void PerformMeleeDamage();

protected:
	UPROPERTY(EditAnywhere, Category="Melee")
	float MeleeDamage = 25.f;

	UPROPERTY(EditAnywhere, Category="Melee")
	float MeleeRange = 120.f;

	UPROPERTY(EditAnywhere, Category="Melee")
	FName MeleeHitSocket = "MeleeSocket";

	/** 히트박스 On 동안 true */
	bool bMeleeHitboxActive = false;

	/** 애니/공격 한 사이클 끝났는지 여부 */
	bool bMeleeFinished = false;

	/** 중복 피격 방지 */
	UPROPERTY()
	TSet<AActor*> MeleeAlreadyHitActors;

	/* ===========================
	 *  Dash Attack (기존 대쉬 공격)
	 * ===========================
	 */
public:
	void PlayDashChargeMontage();
	void BeginDash();
	void EndDash();

	/** Task가 종료를 기다릴 Delegate */
	UPROPERTY(BlueprintAssignable, Category="Boss|Dash")
	FBossDashFinishedDelegate OnDashAttackFinished;

protected:
	/** Dash 이동 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Dash")
	float DashSpeed = 3000.f;

	/** Dash 유지 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Dash")
	float DashDuration = 0.4f;

	/** Dash 공격용 몽타주 (기를 모으는 애니 포함) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boss|Dash")
	UAnimMontage* DashAttackMontage;
	
	
	/** Dash 방향 */
	FVector DashDir = FVector::ZeroVector;

	/* ===========================
	 *  ShortDash (짧은 순간 대쉬)
	 * ===========================
	 */
public:
	/** Task에서 방향/속도/시간을 넘겨서 사용하는 짧은 대쉬 */
	void StartShortDash(const FVector& Dir, float Speed, float Duration);

	UFUNCTION(BlueprintCallable)
	void PlayDashEffects(const FVector& DashDirection);

	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* DashVFX;

	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* GhostTrailVFX;

	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* DashDustVFX;
	// Cascade Particle
	UPROPERTY(EditAnywhere, Category="VFX")
	UParticleSystem* DashParticle;

	UPROPERTY(EditAnywhere, Category="VFX")
	UParticleSystem* GhostTrailParticle;

	UPROPERTY(EditAnywhere, Category="VFX")
	UParticleSystem* DashDustParticle;
	
	UPROPERTY(EditAnywhere, Category="SFX")
	USoundBase* DashSFX;

	UFUNCTION(BlueprintCallable)
	void PlayTakeOffEffects();

	// Niagara
	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* TakeOffDustVFX;

	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* TakeOffShockVFX;

	// Cascade
	UPROPERTY(EditAnywhere, Category="VFX")
	UParticleSystem* TakeOffDustParticle;

	UPROPERTY(EditAnywhere, Category="VFX")
	UParticleSystem* TakeOffShockParticle;

	// SFX
	UPROPERTY(EditAnywhere, Category="SFX")
	USoundBase* TakeOffSFX;
	
protected:
	bool bIsShortDashing = false;
	FVector ShortDashDir = FVector::ZeroVector;
	float ShortDashSpeed = 0.f;
	float ShortDashDuration = 0.f;
	float ShortDashTime = 0.f;

	/* ===========================
	 *  TakeOff (수직 점프)
	 * ===========================
	 */
public:
	/** 위로 수직 상승하는 동작 (중력 약화 + 상향 속도 부여) */
	void StartTakeOff(float UpSpeed, float Duration);

protected:
	float TakeOffOriginalGravity = 1.f;

	/* ===========================
	 *  컴포넌트 / 스탯 / AI
	 * ===========================
	 */
public:
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

	/** 근접용 블레이드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AEnemyBlade> BladeBP;

	UPROPERTY()
	AEnemyBlade* Blade;

	// === 추가: DashAttack 전용 왼손 Blade ===
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AEnemyBlade> LeftBladeBP;

	UPROPERTY()
	AEnemyBlade* LeftBlade;

protected:
	float CurrentHealth = 0.f;
	float MaxHealth = 0.f;
	TMap<FName,float> BodyPartDamage;
	TMap<FName,float> GroggyThreshold;

	/* ===========================
	 *  애니메이션 몽타주들
	 * ===========================
	 */
public:
	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* BoostMontage;
protected:
	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* TakeOffMontage;
	
	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* MeleeMontage;

	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* ShortDashMontage;

	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* KnockMontage;
	
	UPROPERTY(EditAnywhere, Category = "Animation|Montage")
	UAnimMontage* ExecutionMontage;
	
	UPROPERTY(EditAnywhere, Category="Animation|Montage")
	UAnimMontage* DeathMontage;
};
