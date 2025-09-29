// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameEngineBasicPawn.h"
#include "GameEngineBasicSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class GAMEENGINEBASIC_API AGameEngineBasicSportsCar : public AGameEngineBasicPawn
{
	GENERATED_BODY()
	
public:

	AGameEngineBasicSportsCar();
};
