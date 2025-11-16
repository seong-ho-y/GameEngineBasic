// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/States/S_Boost.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/WingComponent.h"
#include "Component/FuelComponent.h"

void US_Boost::Enter_Implementation(ASpaceCharacter* Character)
{
    PreviousState = Character->GetCurrentState();
    Character->bIsBoosting = true;

    FVector InputDir = Character->GetLastMovementInputVector();
    FVector BoostDir = InputDir.GetSafeNormal();

    Character->GetCharacterMovement()->AddImpulse(BoostDir * Character->BoostStrength, true); 

    Character->GetWingComponent()->StopAll();

    if (Character->BoostMontage) {
        Character->GetMesh()->GetAnimInstance()->Montage_Play(Character->BoostMontage);
        Character->GetWingComponent()->PlayJump();
    }
       
    Character->GetFuelComponent()->Consume(Character->GetFuelComponent()->GetBoostCost());


    Elapsed = 0.f;
}

void US_Boost::Tick_Implementation(ASpaceCharacter* Character, float DeltaTime)
{
    Elapsed += DeltaTime;

    if (Elapsed >= Character->BoostDuration)
    {
        Character->ChangeState(ECharacterState::Locomotion);
    }
}

void US_Boost::Exit_Implementation(ASpaceCharacter* Character)
{
    Character->bIsBoosting = false;

    if (Character->GetWingComponent())
        Character->GetWingComponent()->StopAll();
}