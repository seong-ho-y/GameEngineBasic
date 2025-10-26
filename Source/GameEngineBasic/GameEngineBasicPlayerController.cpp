// Copyright Epic Games, Inc. All Rights Reserved.


#include "GameEngineBasicPlayerController.h"
#include "GameEngineBasicPawn.h"
#include "GameEngineBasicUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Enemy/Public/EnemyPoolManager.h"
#include "Kismet/GameplayStatics.h"

void AGameEngineBasicPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// spawn the UI widget and add it to the viewport
	VehicleUI = CreateWidget<UGameEngineBasicUI>(this, VehicleUIClass);

	check(VehicleUI);

	VehicleUI->AddToViewport();

	// EnemyPoolManager 찾기
	EnemyPoolManager = Cast<AEnemyPoolManager>(UGameplayStatics::GetActorOfClass(GetWorld(),StaticClass()));
	if (!EnemyPoolManager)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyPoolManager not found in the world!"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("EnemyPoolManager successfully found."));
	}
}

void AGameEngineBasicPlayerController::OnLKeyPressed()
{
	UE_LOG(LogTemp, Log, TEXT("L Key Pressed"));
	if (EnemyPoolManager)
	{
		EnemyPoolManager->SpawnEnemiesAtSpawnPoints();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("EnemPoolManager is null"));
	}
}

void AGameEngineBasicPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}
	InputComponent->BindKey(EKeys::L, IE_Pressed, this, &AGameEngineBasicPlayerController::OnLKeyPressed);
}

void AGameEngineBasicPlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
	}
}

void AGameEngineBasicPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	VehiclePawn = CastChecked<AGameEngineBasicPawn>(InPawn);
}
