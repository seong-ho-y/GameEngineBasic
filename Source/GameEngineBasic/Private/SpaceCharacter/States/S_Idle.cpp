// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_Idle.h"
#include "SpaceCharacter/SpaceCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

void US_Idle::Enter_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;
    UCharacterMovementComponent* Move = Character->GetCharacterMovement();
    if (Move)
    {
        Move->SetMovementMode(MOVE_Walking);
        Move->bOrientRotationToMovement = true;
    }
}

void US_Idle::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{
    if (!Character) return;

    // 속도 기반 이동
    FVector Velocity = Character->GetVelocity();
    Velocity.Z = 0.f;

    // Sprint 여부에 따른 속도 조정
    UCharacterMovementComponent* Move = Character->GetCharacterMovement();
    if (Character->bIsSprinting)
        Move->MaxWalkSpeed = Character->RunSpeed;
    else
        Move->MaxWalkSpeed = Character->WalkSpeed;
}

void US_Idle::Exit_Implementation(ASpaceCharacter* Character)
{
    // 필요 시 정리 로직
}