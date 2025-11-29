// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "SpaceCharacter/DeathWidget.h"

#include "SpaceShip/MyTestPawn.h"
#include "SpaceCharacter/SpaceCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "GameMode/MyGameMode.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AMyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void AMyPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OnPlayerPawnChanged.Broadcast(InPawn);

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();

        if (Cast<ASpaceCharacter>(InPawn))
        {
            Subsystem->AddMappingContext(CharacterInputContext, 0);

        }
        else if (Cast<AMyTestPawn>(InPawn))
        {
            Subsystem->AddMappingContext(ShipInputContext, 0);

        }
    }
}

void AMyPlayerController::ShowDeathWidget()
{
    if (!DeathWidgetclass) return;

    DeathWidget = CreateWidget<UDeathWidget>(this, DeathWidgetclass);

    if (DeathWidget)
    {
        DeathWidget->AddToViewport();

        FInputModeUIOnly InputMode;
        
		SetInputMode(InputMode);
        bShowMouseCursor = true;
    }
}

void AMyPlayerController::RequestRespawn()
{
    if (DeathWidget)
    {
        DeathWidget->RemoveFromParent();
        DeathWidget = nullptr;
    }

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());

    if (AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)))
        GM->RespawnPlayer(this);
}
