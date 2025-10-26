// Fill out your copyright notice in the Description page of Project Settings.

#include "MyPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "GameEngineBasic/Enemy/Public/EnemyPoolManager.h"
#include "Kismet/GameplayStatics.h"
#include "SpaceShip/MySpaceShip.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// EnemyPoolManager 찾기
	EnemyPoolManager = Cast<AEnemyPoolManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemyPoolManager::StaticClass()));
	if (!EnemyPoolManager)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyPoolManager not found in the world!"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("EnemyPoolManager successfully found."));
	}
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

	//L키 바인딩
	InputComponent->BindKey(EKeys::L, IE_Pressed, this, &AMyPlayerController::OnLKeyPressed);
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
void AMyPlayerController::OnLKeyPressed()
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