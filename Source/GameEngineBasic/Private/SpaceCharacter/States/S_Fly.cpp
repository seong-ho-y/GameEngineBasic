// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_Fly.h"

#include "SpaceCharacter/SpaceCharacter.h"
#include "Component/FuelComponent.h"
#include "GameFramework/CharacterMovementComponent.h"



void US_Fly::Enter_Implementation(ASpaceCharacter* Character)
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Enter Fly State"));
	}
	if (!Character) return;
	auto* Move = Character->GetCharacterMovement();
	if (!Move) return;

	Character->bIsFlyingMode = true;
	Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	Character->bUseControllerRotationYaw = false;

	Move->SetMovementMode(MOVE_Flying);
	Move->GravityScale = 0.05f;
	Move->BrakingFrictionFactor = 15.f;
	Move->AirControl = 1.f;
}

void US_Fly::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{
	if (!Character) return;
	auto* Move = Character->GetCharacterMovement();
	Character->GetFuelComponent()->ConsumeFlying(DeltaTime);

	if (Move->MovementMode != MOVE_Flying)
	{
		Move->SetMovementMode(MOVE_Flying);
		Move->GravityScale = 0.05f;
		Move->BrakingFrictionFactor = 10.f; // Enter에서 설정한 값과 동일하게
		Move->AirControl = 1.f;
	}

	if (!Character->GetFuelComponent()->CanFly())
	{
		Character->ChangeState(ECharacterState::Locomotion);
	}
}

void US_Fly::Exit_Implementation(ASpaceCharacter* Character)
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Exit Fly State"));
	}
	if (!Character) return;
	auto* Move = Character->GetCharacterMovement();
	if (!Move) return;

	Character->bIsFlyingMode = false;
	Move->SetMovementMode(MOVE_Walking);
	Move->GravityScale = 1.f;
	Move->BrakingFrictionFactor = 2.f;
	Move->AirControl = 0.2f;
}