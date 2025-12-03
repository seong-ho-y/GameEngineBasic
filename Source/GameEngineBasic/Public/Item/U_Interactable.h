// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "U_Interactable.generated.h"


class ASpaceCharacter;

UINTERFACE(Blueprintable)
class UU_Interactable : public UInterface
{
	GENERATED_BODY()
};

class GAMEENGINEBASIC_API IU_Interactable
{
	GENERATED_BODY()

public:
	virtual void Interact(ASpaceCharacter* Character) = 0;
};
