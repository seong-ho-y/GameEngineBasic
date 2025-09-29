// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameEngineBasicGameMode.h"
#include "GameEngineBasicPlayerController.h"

AGameEngineBasicGameMode::AGameEngineBasicGameMode()
{
	PlayerControllerClass = AGameEngineBasicPlayerController::StaticClass();
}
