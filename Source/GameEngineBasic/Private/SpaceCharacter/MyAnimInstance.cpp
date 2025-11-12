// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/MyAnimInstance.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include <KismetAnimationLibrary.h>

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwningCharacter = Cast<ASpaceCharacter>(TryGetPawnOwner());

	bIsBoosting = false;
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwningCharacter)
	{
		AnimState = OwningCharacter->GetCurrentState();

		FVector Velocity = OwningCharacter->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();

		bIsInAir = OwningCharacter->GetCharacterMovement()->IsFalling();
		bisAccelerating = OwningCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
		bIsFlying = OwningCharacter->GetCharacterMovement()->MovementMode == MOVE_Flying;


		FRotator ActorRot = OwningCharacter->GetActorRotation();
		AimRotation = OwningCharacter->GetBaseAimRotation();
		FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, ActorRot);

		Pitch = FMath::FInterpTo(Pitch, DeltaRot.Pitch, DeltaSeconds, 15.0f);
		Yaw = FMath::FInterpTo(Yaw, DeltaRot.Yaw, DeltaSeconds, 15.0f);

		FRotator VelRot = UKismetMathLibrary::MakeRotFromX(Velocity);
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, ActorRot);
		MovementRotation = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot);
		

		bIsBoosting = OwningCharacter->bIsBoosting;
		bIsAiming = OwningCharacter->bIsAiming;
	}
}