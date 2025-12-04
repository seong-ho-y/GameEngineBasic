// EnemyAnimInstance.cpp
#include "EnemyAnimInstance.h"

#include "EnemyHuman.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerChar = Cast<ACharacter>(TryGetPawnOwner());
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwnerChar)
	{
		OwnerChar = Cast<ACharacter>(TryGetPawnOwner());
		if (!OwnerChar || !OwnerChar->GetCharacterMovement()) return;
	}

	UpdateState();
	UpdateLocomotionParams(DeltaSeconds);
	UpdateAimParams(DeltaSeconds);
	// ===============================
	//     ★ 공중 Boost 몽타주 처리 ★
	// ===============================
	AEnemyHuman* Enemy = Cast<AEnemyHuman>(OwnerChar);
	if (!Enemy) return;

	UCharacterMovementComponent* Move = Enemy->GetCharacterMovement();
	if (!Move) return;

	// 공중 상태 기준
	const bool bFalling = Move->IsFalling();

	if (bFalling && !bBoostMontagePlaying)
	{
		if (Enemy->BoostMontage)
		{
			Montage_Play(Enemy->BoostMontage);
			bBoostMontagePlaying = true;
		}
	}
	else if (!bFalling && bBoostMontagePlaying)
	{
		Montage_Stop(0.2f);
		bBoostMontagePlaying = false;
	}
}

void UEnemyAnimInstance::UpdateState()
{
	if (!OwnerChar) return;
	AEnemyHuman* Owner = Cast<AEnemyHuman>(OwnerChar);
	// --- 전신 상태 우선 (가장 높은 우선순위) ---
	if (Owner->bIsDead)
	{
		FullBodyState = EFullBodyState::Dead;
		return;
	}
	if (Owner->bIsExecuting)
	{
		FullBodyState = EFullBodyState::Execution;
		return;
	}
	if (Owner->bIsKnocked)
	{
		FullBodyState = EFullBodyState::Knock;
		return;
	}
	if (bIsDashAttacking)
	{
		FullBodyState = EFullBodyState::DashAttack;
		return;
	}
	else
	{
		FullBodyState = EFullBodyState::Default;
	}

	// --- 하체 상태 ---
	if (bIsBoosting)
		LowerBodyState = ELowerBodyState::Boost;
	else
		LowerBodyState = ELowerBodyState::WalkBlendSpace;


	// --- 상체 상태 ---
	if (bIsMeleeAttacking)
		UpperBodyState = EUpperBodyState::Melee;
	else if (bShooting)
		UpperBodyState = EUpperBodyState::Shoot;
	else if (bReloading)
		UpperBodyState = EUpperBodyState::Reload;
	else if (bAiming)
		UpperBodyState = EUpperBodyState::Aim;
	else
		UpperBodyState = EUpperBodyState::Idle;
}

void UEnemyAnimInstance::UpdateLocomotionParams(float DeltaSeconds)
{
	const FVector Velocity = OwnerChar->GetVelocity();
	const FVector HorizontalVel = FVector(Velocity.X, Velocity.Y, 0.f);

	const float StopThreshold = 8.0f;
	
	// ===== Interp Speed =====
	float TargetSpeed = HorizontalVel.Size();
	if (TargetSpeed < StopThreshold) TargetSpeed = 0.f;

	Speed = FMath::FInterpTo(Speed, TargetSpeed, DeltaSeconds, 5.f);

	// ---- Standard Vector ----
	const FRotator ActorRot = OwnerChar->GetActorRotation();
	const FVector Forward = UKismetMathLibrary::GetForwardVector(ActorRot);
	const FVector Right = UKismetMathLibrary::GetRightVector(ActorRot);

	// === Calculate RightSpeed ===
	float TargetRightSpeed = FVector::DotProduct(HorizontalVel, Right);

	// Small value -> 0 value
	if (FMath::Abs(TargetRightSpeed) < 5.f || TargetSpeed == 0.f) TargetRightSpeed = 0.f;

	// Interp Smoothly
	RightSpeed = FMath::FInterpTo(RightSpeed, TargetRightSpeed, DeltaSeconds, 5.f);

	bIsInAir = OwnerChar->GetCharacterMovement()->IsFalling();
}

void UEnemyAnimInstance::UpdateAimParams(float DeltaSeconds)
{
	const FRotator ActorRot = OwnerChar->GetActorRotation();

	FRotator ViewRot = ActorRot;
	if (APawn* Pawn = Cast<APawn>(OwnerChar))
	{
		if (AController* C = Pawn->GetController())
		{
			ViewRot = C->GetControlRotation();
		}
	}

	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(ViewRot, ActorRot);
	AimYaw = Delta.Yaw;
	AimPitch = Delta.Pitch;
}

void UEnemyAnimInstance::AnimNotify_KnockEnd()
{
	GEngine->AddOnScreenDebugMessage(52352, 1.f, FColor::Red, TEXT("AnimNotify_KnockEnd fired"));
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(OwnerChar))
	{
		Enemy->bIsKnocked = false;
	}
}
void UEnemyAnimInstance::AnimNotify_MeleeBegin()
{
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(TryGetPawnOwner()))
	{
		Enemy->OnMeleeBegin();
	}
}

void UEnemyAnimInstance::AnimNotify_MeleeEnd()
{
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(TryGetPawnOwner()))
	{
		Enemy->OnMeleeEnd();
	}
}

void UEnemyAnimInstance::AnimNotify_DashStart()
{
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(TryGetPawnOwner()))
	{
		Enemy->BeginDash();
	}
}
void UEnemyAnimInstance::AnimNotify_LeftBladeBegin()
{
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(TryGetPawnOwner()))
	{
		Enemy->OnLeftBladeBegin();
	}
}
void UEnemyAnimInstance::AnimNotify_LeftBladeEnd()
{
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(TryGetPawnOwner()))
	{
		Enemy->OnLeftBladeEnd();
	}
}
void UEnemyAnimInstance::AnimNotify_BladeAppear()
{
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(TryGetPawnOwner()))
	{
		Enemy->PlayBladeAppearSFX();
	}
}
void UEnemyAnimInstance::AnimNotify_BladeSwing()
{
	if (AEnemyHuman* Enemy = Cast<AEnemyHuman>(TryGetPawnOwner()))
	{
		Enemy->PlayBladeSwingSFX();
	}
}