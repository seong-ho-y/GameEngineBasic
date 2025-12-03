// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_FlyCharge.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"

void US_FlyCharge::Enter_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

    if (!Character->bIsAiming)
    {
        Character->ChangeState(ECharacterState::Flying);
        return;
    }

    auto ShooterComp = Character->GetShooterComponent();
    auto FollowCam = Character->GetFollowCameraComponent();
    if (!ShooterComp || !FollowCam)
    {
        Character->ChangeState(ECharacterState::FlyAim);
        return;
    }

    // --- ForwardVector 기반 즉시 발사 ---
    FVector CameraLoc = FollowCam->GetComponentLocation();
    FVector CameraDir = FollowCam->GetForwardVector();
    FVector TraceEnd = CameraLoc + CameraDir * 10000.f;

    FHitResult Hit;
    if (Character->GetWorld()->LineTraceSingleByChannel(
        Hit, CameraLoc, TraceEnd, ECC_Visibility))
    {
        TraceEnd = Hit.ImpactPoint;
    }

    FVector MuzzleLoc = Character->GetMesh()->GetSocketLocation(TEXT("Muzzle"));
    FVector FireDir = (TraceEnd - MuzzleLoc).GetSafeNormal();

    ShooterComp->SetFireDirection(FireDir);
    ShooterComp->PendingDamage = 20.f;   // 필요시 조정
    ShooterComp->PendingScale = 1.f;     // 필요시 조정
    ShooterComp->TryFire();


    Character->ChangeState(ECharacterState::FlyAim);
}


void US_FlyCharge::Exit_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;
}