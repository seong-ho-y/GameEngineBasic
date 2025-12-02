// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MyGameMode.h"

#include "SpaceCharacter/SpaceCharacter.h"
#include "GameEngineBasic/System/SaveSystemManager.h"

#include "MyPlayerHUD.h"
#include "MyPlayerController.h"
#include <Kismet/GameplayStatics.h>
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Component/ShieldComp.h"

AMyGameMode::AMyGameMode()
{
	DefaultPawnClass = ASpaceCharacter::StaticClass();
	PlayerControllerClass = AMyPlayerController::StaticClass();
}

void AMyGameMode::BeginPlay()
{
	Super::BeginPlay();

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    ASpaceCharacter* Char = Cast<ASpaceCharacter>(PC->GetPawn());
    if (!Char) return;

    FVector SpawnLoc;
    FRotator SpawnRot;

    bool bIsRespawn = UGameplayStatics::HasOption(OptionsString, TEXT("IsRespawn"));

    // 1) 세이브된 Bonfire가 있으면 그 위치로
    if (bIsRespawn && USaveSystemManager::GetSavedSpawnPoint(SpawnLoc, SpawnRot))
    {
        Char->SetActorLocation(SpawnLoc);
        Char->SetActorRotation(SpawnRot);
    }
    // 2) 없으면 DefaultSpawnPoint로
    else if (DefaultSpawnPoint)
    {
        Char->SetActorLocation(DefaultSpawnPoint->GetActorLocation());
        Char->SetActorRotation(DefaultSpawnPoint->GetActorRotation());
    }

    // 3) 항상 풀피로 시작
    if (auto Health = Char->GetHealthComponent())
    {
        Health->CurrentHealth = Health->MaxHealth;
        Health->OnHealthChanged_Ver2.Broadcast(Health->CurrentHealth, Health->MaxHealth);
    }

    if (auto Shield = Char->GetShieldComponent())
    {
        Shield->CurrentShield = Shield->MaxShield;
    }
    /*
    if (APawn* Pawn = PC->GetPawn())
        USaveSystemManager::LoadPawnState(Pawn);
        */
}

void AMyGameMode::RespawnPlayer(AController* Controller)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("RespawnPlayer Called"));

	FName CurrentLevelName(*GetWorld()->GetName());
	UGameplayStatics::OpenLevel(this, CurrentLevelName, true, TEXT("?IsRespawn"));
}