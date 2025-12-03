// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_FlyAim.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"

void US_FlyAim::Enter_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

    Character->bIsAiming = true;
    Character->bIsCameraTransitioning = true;

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;
}

void US_FlyAim::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{
    if (!Character) return;

    // 카메라 에임 시 카메라 보간 유지
    if (Character->bIsCameraTransitioning)
        Character->UpdateCameraTransition(DeltaTime);
}

void US_FlyAim::Exit_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

    Character->bIsAiming = false;

    Character->GetCharacterMovement()->bOrientRotationToMovement = true;
    Character->bUseControllerRotationYaw = false;
}
