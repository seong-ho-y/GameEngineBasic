// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_Idle.h"
#include "SpaceCharacter/SpaceCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

void US_Idle::Enter_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;
    UCharacterMovementComponent* Move = Character->GetCharacterMovement();

    Character->bIsAiming = false;
    Character->GetCharacterMovement()->bOrientRotationToMovement = true;
    Character->bUseControllerRotationYaw = false;

    Character->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    Character->GetCharacterMovement()->GravityScale = 1.f;

    // 카메라 줌 아웃을 위해 트랜지션 시작
    if (Character->GetCameraBoom()->TargetArmLength != Character->DefaultArmLength ||
        Character->GetCameraBoom()->SocketOffset != Character->DefaultSocketOffset)
    {
        Character->bIsCameraTransitioning = true;
    }

    if (Move)
    {
        Move->SetMovementMode(MOVE_Walking);
    }
}

void US_Idle::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{

}

void US_Idle::Exit_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

    Character->bIsAiming = false;

    UCharacterMovementComponent* Move = Character->GetCharacterMovement();
    if (Move)
    {
        Move->SetMovementMode(MOVE_Walking);
        Move->bOrientRotationToMovement = true;
    }
}