// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_Aim.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/KismetMathLibrary.h"


void US_Aim::Enter_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

    bTransitioningCamera = true;
    Character->bIsAiming = true;

    Character->GetCharacterMovement()->bOrientRotationToMovement = false;
    Character->bUseControllerRotationYaw = true;

    Character->bIsCameraTransitioning = true;

}

void US_Aim::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{
    if (!Character) return;

    // 카메라 전환이 끝났는지 체크
    if (Character->bIsCameraTransitioning)
        Character->UpdateCameraTransition(DeltaTime);
}

void US_Aim::Exit_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

}
