// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/System/SaveSystemManager.h"

#include "GameEngineBasic/System/SaveSystem.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameEngineBasic/Public/MyPlayerState.h"
#include "SpaceShip/MyTestPawn.h"

#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Component/ShieldComp.h"
#include "Component/FuelComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "WeaponComponent.h"

void USaveSystemManager::SavePawnState(APawn* Pawn)
{
    if (!Pawn) return;

    USaveSystem* SaveObj = nullptr;
    FString SaveSlotName = TEXT("PlayerSave");
    int32 SaveIndex = 0;

    if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveIndex))
    {
        // 기존 파일 로드
        SaveObj = Cast<USaveSystem>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveIndex));
    }
    else
    {
        // 파일이 없으면 새로 생성
        SaveObj = Cast<USaveSystem>(UGameplayStatics::CreateSaveGameObject(USaveSystem::StaticClass()));
    }

    if (!SaveObj) return; // 방어 코드

    // 공통: 위치 정보
    SaveObj->SavedLocation = Pawn->GetActorLocation();
    SaveObj->SavedRotation = Pawn->GetActorRotation();

	AMyPlayerState* PS = Cast<AMyPlayerState>(Pawn->GetPlayerState());
	if (!PS) return;
    SaveObj->bCanSprint = PS->AbilityStatus.bCanSprint;
    SaveObj->bCanDash = PS->AbilityStatus.bCanDash;
    SaveObj->bCanFly = PS->AbilityStatus.bCanFly;
    SaveObj->bCanShield = PS->AbilityStatus.bCanShield;

    SaveObj->WeaponRowName = PS->UnlockStatus.UnlockedWeapons.Array();

    // *** SpaceCharacter ***
    if (ASpaceCharacter* Char = Cast<ASpaceCharacter>(Pawn))
    {
        if (Char->GetShooterComponent())
        {
            SaveObj->CurrentAmmo = Char->GetShooterComponent()->MaxAmmo;
            SaveObj->FullAmmo = Char->GetShooterComponent()->FullAmmo;
            SaveObj->MaxAmmo = Char->GetShooterComponent()->MaxAmmo;
        }
    }

    // *** MyTestPawn (우주선) ***
    else if (AMyTestPawn* Ship = Cast<AMyTestPawn>(Pawn))
    {
        SaveObj->CurrentHealth = Ship->GetHealthComponent()->CurrentHealth;
        SaveObj->MaxHealth = Ship->GetHealthComponent()->MaxHealth;

        SaveObj->CurrentShield = Ship->GetShieldComponent()->CurrentShield;
        SaveObj->MaxShield = Ship->GetShieldComponent()->MaxShield;

        if (Ship->GetShooterComponent())
        {
            SaveObj->CurrentAmmo = Ship->GetShooterComponent()->CurrentAmmo;
            SaveObj->FullAmmo = Ship->GetShooterComponent()->FullAmmo;
            SaveObj->MaxAmmo = Ship->GetShooterComponent()->MaxAmmo;
        }
    }

    // Save!
    UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("PlayerSave"), 0);
}

void USaveSystemManager::LoadPawnState(APawn* Pawn)
{
    if (!Pawn) return;

    if (!UGameplayStatics::DoesSaveGameExist(TEXT("PlayerSave"), 0))
        return;

    USaveSystem* SaveObj = Cast<USaveSystem>(
        UGameplayStatics::LoadGameFromSlot(TEXT("PlayerSave"), 0)
    );

    AMyPlayerState* PS = Cast<AMyPlayerState>(Pawn->GetPlayerState());
    if (PS)
    {
        PS->AbilityStatus.bCanSprint = SaveObj->bCanSprint;
        PS->AbilityStatus.bCanDash = SaveObj->bCanDash;
        PS->AbilityStatus.bCanFly = SaveObj->bCanFly;
        PS->AbilityStatus.bCanShield = SaveObj->bCanShield;

        for (const FName& WeaponName : SaveObj->WeaponRowName)
        {
            PS->UnlockStatus.UnlockedWeapons.Add(WeaponName);
        }
    }
    
    //Pawn->SetActorLocation(SaveObj->SavedLocation);
    //Pawn->SetActorRotation(SaveObj->SavedRotation);

    // SpaceCharacter
    if (ASpaceCharacter* Char = Cast<ASpaceCharacter>(Pawn))
    {
        if (Char->GetShooterComponent())
        {
            Char->GetShooterComponent()->CurrentAmmo = SaveObj->CurrentAmmo;
            Char->GetShooterComponent()->FullAmmo = SaveObj->FullAmmo;
            Char->GetShooterComponent()->MaxAmmo = SaveObj->MaxAmmo;
        }

        if (Char->GetWeaponComponent())
        {
            Char->GetWeaponComponent()->InitializeWeapon(Char, Char->GetShooterComponent());
        }
    }

    // MyTestPawn (우주선)
    else if (AMyTestPawn* Ship = Cast<AMyTestPawn>(Pawn))
    {
        Ship->GetHealthComponent()->CurrentHealth = SaveObj->CurrentHealth;
        Ship->GetHealthComponent()->MaxHealth = SaveObj->MaxHealth;

        Ship->GetShieldComponent()->CurrentShield = SaveObj->CurrentShield;
        Ship->GetShieldComponent()->MaxShield = SaveObj->MaxShield;

        if (Ship->GetShooterComponent())
        {
            Ship->GetShooterComponent()->CurrentAmmo = SaveObj->CurrentAmmo;
            Ship->GetShooterComponent()->FullAmmo = SaveObj->FullAmmo;
            Ship->GetShooterComponent()->MaxAmmo = SaveObj->MaxAmmo;
        }
    }
}

void USaveSystemManager::SaveSpawnPoint(const FVector& Location, const FRotator& Rotation)
{
    if (!UGameplayStatics::DoesSaveGameExist(TEXT("PlayerSave"), 0))
        return;

    USaveSystem* SaveObj = Cast<USaveSystem>(
        UGameplayStatics::LoadGameFromSlot(TEXT("PlayerSave"), 0)
    );

    SaveObj->SavedLocation = Location;
    SaveObj->SavedRotation = Rotation;

    UGameplayStatics::SaveGameToSlot(SaveObj, TEXT("PlayerSave"), 0);
}

bool USaveSystemManager::GetSavedSpawnPoint(FVector& OutLoc, FRotator& OutRot)
{
    if (!UGameplayStatics::DoesSaveGameExist(TEXT("PlayerSave"), 0))
        return false;

    USaveSystem* SaveObj = Cast<USaveSystem>(
        UGameplayStatics::LoadGameFromSlot(TEXT("PlayerSave"), 0)
    );

    OutLoc = SaveObj->SavedLocation;
    OutRot = SaveObj->SavedRotation;
    return true;
}
