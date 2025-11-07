// EnemyAnimInstance.cpp
#include "EnemyAnimInstance.h"

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

	UpdateLocomotionParams(DeltaSeconds);
	UpdateAimParams(DeltaSeconds);
}

void UEnemyAnimInstance::UpdateState()
{
	if (!OwnerChar) return;

	// --- 전신 상태 우선 (가장 높은 우선순위) ---
	if (bIsDead)
	{
		FullBodyState = EFullBodyState::Dead;
		return;
	}
	else if (bIsKnocked)
	{
		FullBodyState = EFullBodyState::Knock;
		return;
	}
	else
	{
		FullBodyState = EFullBodyState::Default;
	}

	// --- 하체 상태 ---
	if (bIsBoosting)
		LowerBodyState = ELowerBodyState::Boost;
	else if (Speed > 10.f)
		LowerBodyState = ELowerBodyState::Walk;
	else
		LowerBodyState = ELowerBodyState::Idle;

	// --- 상체 상태 ---
	if (bShooting)
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

	float NewSpeed = HorizontalVel.Size();
	float SmoothSpeed = NewSpeed;

	const float StopThreshold = 5.0f;

	if (NewSpeed < StopThreshold)
	{
		SmoothSpeed = FMath::FInterpTo(Speed, 0.f, DeltaSeconds, 5.f);
	}
	else
	{
		SmoothSpeed = FMath::FInterpTo(Speed, NewSpeed, DeltaSeconds, 10.f);
	}
	Speed   = SmoothSpeed;
	bIsInAir = OwnerChar->GetCharacterMovement()->IsFalling();

	// 전/측면 분해: 캐릭터 기준 전/우 벡터에 투영
	const FRotator ActorRot = OwnerChar->GetActorRotation();
	const FVector Forward = UKismetMathLibrary::GetForwardVector(ActorRot);
	const FVector Right   = UKismetMathLibrary::GetRightVector(ActorRot);

	ForwardSpeed = FVector::DotProduct(HorizontalVel, Forward); // +전진 / -후진
	RightSpeed   = FVector::DotProduct(HorizontalVel, Right);   // +오른쪽 / -왼쪽

	// 방향(각도)도 필요하면: -180~180
	Direction = UKismetAnimationLibrary::CalculateDirection(HorizontalVel, ActorRot);
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
