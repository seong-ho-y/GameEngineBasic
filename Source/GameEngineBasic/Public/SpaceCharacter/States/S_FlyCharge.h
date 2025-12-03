// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpaceCharacter/States/CharacterStateBase.h"
#include "S_FlyCharge.generated.h"

UCLASS()
class GAMEENGINEBASIC_API US_FlyCharge : public UCharacterStateBase
{
	GENERATED_BODY()

public:
    virtual void Enter_Implementation(ASpaceCharacter* Character) override;
    virtual void Exit_Implementation(ASpaceCharacter* Character) override;
};
