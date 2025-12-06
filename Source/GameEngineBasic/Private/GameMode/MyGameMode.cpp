// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MyGameMode.h"

#include "SpaceCharacter/SpaceCharacter.h"
#include "GameEngineBasic/System/SaveSystemManager.h"

#include "MyPlayerHUD.h"
#include "MyPlayerController.h"
#include <Kismet/GameplayStatics.h>
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Component/ShieldComp.h"
#include "MyPlayerState.h"

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

    bool bIsRespawn = UGameplayStatics::HasOption(OptionsString, TEXT("IsRespawn"));
    bool bIsStageTransition = UGameplayStatics::HasOption(OptionsString, TEXT("IsStageTransition"));

    // 1) 세이브된 Bonfire가 있으면 그 위치로
    if (bIsRespawn)
    {
        // 1. 리스폰 로직: 저장된 위치와 상태를 불러옴
        FVector SpawnLoc;
        FRotator SpawnRot;
        if (USaveSystemManager::GetSavedSpawnPoint(SpawnLoc, SpawnRot))
        {
            Char->SetActorLocation(SpawnLoc);
            Char->SetActorRotation(SpawnRot);
        }
        USaveSystemManager::LoadPawnState(Char);
    }
	// 2) Level Transition 
    else if (bIsStageTransition)
    {
        if (DefaultSpawnPoint)
        {
            Char->SetActorLocation(DefaultSpawnPoint->GetActorLocation());
            Char->SetActorRotation(DefaultSpawnPoint->GetActorRotation());
        }

        USaveSystemManager::LoadPawnState(Char);
    }
	// 3) New Game
    else
    {
        if (DefaultSpawnPoint)
        {
            Char->SetActorLocation(DefaultSpawnPoint->GetActorLocation());
            Char->SetActorRotation(DefaultSpawnPoint->GetActorRotation());
        }

        AMyPlayerState* PS = Cast<AMyPlayerState>(PC->PlayerState);
        if (PS)
        {
            PS->AbilityStatus.bCanSprint = false;
            PS->AbilityStatus.bCanDash = false;
            PS->AbilityStatus.bCanFly = false;
            PS->AbilityStatus.bCanShield = false;
            PS->AbilityStatus.bCanBoost = false;

            PS->UnlockStatus.UnlockedWeapons.Empty();
            PS->UnlockWeapon("HandGunBasic");
        }
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

void AMyGameMode::RequestStageTransition(FName TargetStageName)
{
    if (TargetStageName.IsNone())
        return;
    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        if (ASpaceCharacter* Char = Cast<ASpaceCharacter>(PC->GetPawn()))
        {
            USaveSystemManager::SaveAbilities(Char);   // 능력 저장
            USaveSystemManager::SaveWeapons(Char);     // 무기 저장
        }
    }
    UGameplayStatics::OpenLevel(GetWorld(), TargetStageName, true, TEXT("?IsStageTransition"));
}
