// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "SpaceShip/MySpaceShip.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
}

void AMyPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AMySpaceShip* Ship = Cast<AMySpaceShip>(InPawn))
	{
		return;
	}
}
