// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ELowerBodyState : uint8
{
	WalkBlendSpace,
	Boost,
	Jump,
	ShortDash,
	Land
};

UENUM(BlueprintType)
enum class EUpperBodyState : uint8
{
	Idle,
	Aim,
	Shoot,
	Reload,
	Melee,
};

UENUM(BlueprintType)
enum class EFullBodyState : uint8
{
	Default,
	Knock,
	DashAttack,
	Execution,
	Dead
};


UCLASS()
class GAMEENGINEBASIC_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ELowerBodyState LowerBodyState = ELowerBodyState::WalkBlendSpace;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EUpperBodyState UpperBodyState = EUpperBodyState::Idle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EFullBodyState FullBodyState = EFullBodyState::Default;
	
	bool bShooting;
	bool bIsShortDashing = false;
	bool bIsJumping = false;
	bool bReloading;
	bool bAiming;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	void UpdateState();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Aim")
	float TargetUpperYaw = 0.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Aim")
	float TargetUpperPitch = 0.f;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float Direction = 0.f;             // 전/후/좌/우 방향(각도, -180~180)

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float ForwardSpeed = 0.f;          // 전/후 성분(+전진, -후진)

	UPROPERTY(BlueprintReadOnly, Category="Movement")
	float RightSpeed = 0.f;            // 좌/우 성분(+오른쪽, -왼쪽)

	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsInAir = false;
public:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsMeleeAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsDashAttacking = false;

	UPROPERTY()
	bool bBoostMontagePlaying = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsKicking = false;
	
	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsAiming = false;

	UPROPERTY(BlueprintReadOnly, Category="State")
	bool bIsFiring = false;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsBoosting = false;
	
	/** ===== 에임(상체) 파라미터 (AimOffset 등) ===== */
	UPROPERTY(BlueprintReadOnly, Category="Aiming")
	float AimYaw = 0.f;

	UPROPERTY(BlueprintReadOnly, Category="Aiming")
	float AimPitch = 0.f;

	/** ===== 몽타주 (에디터에서 지정) ===== */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	UAnimMontage* FireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Montage")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	UAnimMontage* BoostMontage;

	/** C++에서 외부(Enemy/AI)에서 호출할 헬퍼 */
	UFUNCTION(BlueprintCallable, Category="Anim")
	void SetAiming(bool bNewAiming) { bIsAiming = bNewAiming; }

	UFUNCTION(BlueprintCallable, Category="Anim")
	void SetFiring(bool bNewFiring) { bIsFiring = bNewFiring; }

private:
	/** 내부 계산용 */
	UPROPERTY(Transient)
	ACharacter* OwnerChar = nullptr;

	void UpdateLocomotionParams(float DeltaSeconds);
	void UpdateAimParams(float DeltaSeconds);
	UFUNCTION()
	void AnimNotify_KnockEnd();
	UFUNCTION()
	void AnimNotify_MeleeBegin();

	UFUNCTION()
	void AnimNotify_MeleeEnd();
	UFUNCTION()
	void AnimNotify_DashStart();
	UFUNCTION()
	void AnimNotify_LeftBladeBegin();
	UFUNCTION()
	void AnimNotify_LeftBladeEnd();
};