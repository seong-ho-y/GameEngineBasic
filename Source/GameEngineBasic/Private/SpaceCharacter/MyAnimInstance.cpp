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
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (!OwningCharacter)
	{
		OwningCharacter = Cast<ASpaceCharacter>(TryGetPawnOwner());
	}
	if (OwningCharacter)
	{
		FVector Velocity = OwningCharacter->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();

		bIsInAir = OwningCharacter->GetCharacterMovement()->IsFalling();
		bisAccelerating = OwningCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
		bIsFlying = OwningCharacter->GetCharacterMovement()->MovementMode == MOVE_Flying;
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwningCharacter->GetActorRotation());

		FRotator ActorRot = OwningCharacter->GetActorRotation();
		FRotator VelRot = UKismetMathLibrary::MakeRotFromX(Velocity);
		MovementRotation = UKismetMathLibrary::NormalizedDeltaRotator(VelRot, ActorRot);

		AimRotation = OwningCharacter->GetBaseAimRotation();
	}
}