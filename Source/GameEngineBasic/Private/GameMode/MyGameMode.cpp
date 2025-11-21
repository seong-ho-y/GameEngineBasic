// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MyGameMode.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "MyPlayerHUD.h"
#include "MyPlayerController.h"

AMyGameMode::AMyGameMode()
{
	DefaultPawnClass = ASpaceCharacter::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
}