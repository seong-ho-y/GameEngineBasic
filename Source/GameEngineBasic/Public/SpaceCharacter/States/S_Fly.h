// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpaceCharacter/States/CharacterStateBase.h"
#include "S_Fly.generated.h"

class ASpaceCharacter;
class AFuelComponent;

UCLASS()
class GAMEENGINEBASIC_API US_Fly : public UCharacterStateBase
{
	GENERATED_BODY()
	
public:
	virtual void Enter_Implementation(ASpaceCharacter* Character) override;
	virtual void Tick_Implementation(ASpaceCharacter* Character, float DeltaTime) override;
	virtual void Exit_Implementation(ASpaceCharacter* Character) override;

};
