// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpaceCharacter/States/CharacterStateBase.h"
#include "Particles/ParticleSystem.h"
#include "S_Charging.generated.h"


UCLASS()
class GAMEENGINEBASIC_API US_Charging : public UCharacterStateBase
{
    GENERATED_BODY()

public:
    virtual void Enter_Implementation(ASpaceCharacter* Character) override;
    virtual void Tick_Implementation(ASpaceCharacter* Character, float DeltaTime) override;
    virtual void Exit_Implementation(ASpaceCharacter* Character) override;

private:
    float ChargeStartTime = 0.f;
    bool bInputLocked = false;
    bool bExploded = false;

public:

    UPROPERTY(EditAnywhere, Category = "Effects")
    UParticleSystem* ExplosionEffect;
};
