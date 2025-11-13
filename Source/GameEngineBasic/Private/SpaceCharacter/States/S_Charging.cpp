// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_Charging.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameEngineBasic/Components/public/HealthComp.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMeshSocket.h"

void US_Charging::Enter_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;


    if (!Character->bIsAiming)
    {
        Character->ChangeState(ECharacterState::Locomotion);
        return;
    }

    bExploded = false;
    bInputLocked = false;

    ChargeStartTime = Character->GetWorld()->GetTimeSeconds();
    Character->bIsCharging = true;
    Character->CurrentChargeTime = 0.f;


    if (Character->ChargingEffect)
    {
        Character->ActiveChargeEffect = UGameplayStatics::SpawnEmitterAttached(
            Character->ChargingEffect,
            Character->GetMesh(),
            TEXT("Muzzle"),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }

    UE_LOG(LogTemp, Warning, TEXT("Charge Started"));
}

void US_Charging::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{
    if (!Character || !Character->bIsCharging) return;

    const float Elapsed = Character->GetWorld()->GetTimeSeconds() - ChargeStartTime;
    Character->CurrentChargeTime = Elapsed;

    const float ClampedRatio = FMath::Clamp(Elapsed / Character->MaxChargeTime, 0.f, 1.f);

    if (Character->ActiveChargeEffect)
    {
        const float Scale = FMath::Lerp(0.5f, 3.0f, ClampedRatio);
        Character->ActiveChargeEffect->SetWorldScale3D(FVector(Scale));
    }

    if (Elapsed >= 2.5f && !bInputLocked)
    {
        Character->bIsCameraTransitioning = false; // 카메라 전환 중지
        if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
        {
            PC->SetIgnoreMoveInput(true);
            PC->SetIgnoreLookInput(true);
        }

        if (LockMontage)
        {
            UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
            if (AnimInstance)
            {
                AnimInstance->Montage_Play(LockMontage);
                UE_LOG(LogTemp, Warning, TEXT("Lock Montage Played"));
            }
        }

        bInputLocked = true;
        UE_LOG(LogTemp, Warning, TEXT("Input Locked & Camera Frozen at 3s"));
    }

    if (Elapsed >= 4.5f && !bExploded)
    {
        bExploded = true;
        Character->bIsCharging = false;

        FVector ExplosionLocation = Character->GetActorLocation();
        if (ExplosionEffect)
        {
            UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
            AnimInstance->Montage_Play(DamagedMontage);
            UGameplayStatics::SpawnEmitterAtLocation(Character->GetWorld(), ExplosionEffect, ExplosionLocation);
        }

        if (UHealthComp* Health = Character->FindComponentByClass<UHealthComp>())
        {
            Health->TakeDamage(10); // 피해량 조정 가능
        }

        UE_LOG(LogTemp, Warning, TEXT("Player exploded after overcharge!"));
        Character->ChangeState(ECharacterState::Locomotion);

    }

    if (Elapsed >= Character->MaxChargeTime)
    {
        Character->CurrentChargeTime = Character->MaxChargeTime;
    }
}

void US_Charging::Exit_Implementation(ASpaceCharacter* Character)
{
    if (!Character) return;

    if (APlayerController* PC = Cast<APlayerController>(Character->GetController()))
    {
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
    }

    Character->bIsCharging = false;

    if (Character->ActiveChargeEffect)
    {
        Character->ActiveChargeEffect->DeactivateSystem();
        Character->ActiveChargeEffect = nullptr;
    }

    if (bExploded) {
        return;
    }

    const float Elapsed = Character->GetWorld()->GetTimeSeconds() - ChargeStartTime;
    const float ChargeTime = FMath::Clamp(Elapsed, 0.f, Character->MaxChargeTime);
    const float ChargeRatio = ChargeTime / Character->MaxChargeTime;

    auto ShooterComp = Character->GetShooterComponent();
    auto FollowCam = Character->GetFollowCameraComponent();
    if (!ShooterComp || !FollowCam) return;

    ShooterComp->PendingDamage = FMath::Lerp(5.f, 50.f, ChargeRatio);
    ShooterComp->PendingScale = FMath::Lerp(1.f, 8.f, ChargeRatio);

    FVector CameraLoc = FollowCam->GetComponentLocation();
    FVector CameraDir = FollowCam->GetForwardVector();
    FVector TraceEnd = CameraLoc + (CameraDir * 10000.f);
    FHitResult Hit;
    if (Character->GetWorld()->LineTraceSingleByChannel(Hit, CameraLoc, TraceEnd, ECC_Visibility))
        TraceEnd = Hit.ImpactPoint;

    FVector MuzzleLoc = Character->GetMesh()->GetSocketLocation(TEXT("Muzzle"));
    FVector FireDir = (TraceEnd - MuzzleLoc).GetSafeNormal();
    ShooterComp->SetFireDirection(FireDir);
    ShooterComp->TryFire();
}
