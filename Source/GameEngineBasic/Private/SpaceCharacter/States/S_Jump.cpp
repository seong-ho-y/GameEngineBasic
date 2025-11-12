// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_Jump.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SpaceCharacter/MyAnimInstance.h"

void US_Jump::Enter_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

    auto* Move = Character->GetCharacterMovement();
    if (!Move) return;

    Move->SetMovementMode(MOVE_Falling);
    Character->Jump();
}

void US_Jump::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{
    if (!Character) return;

    auto* Move = Character->GetCharacterMovement();
    if (!Move) return;

    // 착지 감지
    if (Move->IsMovingOnGround())
    {
        Character->ChangeState(ECharacterState::Locomotion);
        return;
    }

    // 공중 제어
    FVector InputDir = Character->GetLastMovementInputVector();
    if (!InputDir.IsNearlyZero())
        Character->AddMovementInput(InputDir, 0.2f);
}

void US_Jump::Exit_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;
    Character->StopJumping();
}