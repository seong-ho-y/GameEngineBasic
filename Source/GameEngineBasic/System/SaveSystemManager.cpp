// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/System/SaveSystemManager.h"

#include "GameEngineBasic/System/SaveSystem.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "SpaceShip/MyTestPawn.h"

#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Component/ShieldComp.h"
#include "Component/FuelComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "WeaponComponent.h"

void USaveSystemManager::SavePawnState(APawn* Pawn)
{
    if (!Pawn) return;

    USaveSystem* SaveObj = Cast<USaveSystem>(
        UGameplayStatics::CreateSaveGameObject(USaveSystem::StaticClass())
    );

    // 공통: 위치 정보
    SaveObj->SavedLocation = Pawn->GetActorLocation();
    SaveObj->SavedRotation = Pawn->GetActorRotation();

    // *** SpaceCharacter ***
    if (ASpaceCharacter* Char = Cast<ASpaceCharacter>(Pawn))
    {
        SaveObj->bCanSprint = Char->bCanSprint;
        SaveObj->bCanDash = Char->bCanDash;
        SaveObj->bCanFly = Char->bCanFly;
        SaveObj->bCanShield = Char->bCanShield;

        if (Char->GetShooterComponent())
        {
            SaveObj->CurrentAmmo = Char->GetShooterComponent()->CurrentAmmo;
            SaveObj->FullAmmo = Char->GetShooterComponent()->FullAmmo;
            SaveObj->MaxAmmo = Char->GetShooterComponent()->MaxAmmo;
        }
        if (Char->GetWeaponComponent())
        {
            SaveObj->WeaponRowName = Char->GetWeaponComponent()->WeaponRowName;
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

    Pawn->SetActorLocation(SaveObj->SavedLocation);
    Pawn->SetActorRotation(SaveObj->SavedRotation);

    // SpaceCharacter
    if (ASpaceCharacter* Char = Cast<ASpaceCharacter>(Pawn))
    {
        Char->bCanSprint = SaveObj->bCanSprint;
        Char->bCanDash = SaveObj->bCanDash;
        Char->bCanFly = SaveObj->bCanFly;
        Char->bCanShield = SaveObj->bCanShield;

        if (Char->GetShooterComponent())
        {
            Char->GetShooterComponent()->CurrentAmmo = SaveObj->CurrentAmmo;
            Char->GetShooterComponent()->FullAmmo = SaveObj->FullAmmo;
            Char->GetShooterComponent()->MaxAmmo = SaveObj->MaxAmmo;
        }

        if (Char->GetWeaponComponent())
        {
            Char->GetWeaponComponent()->WeaponRowName = SaveObj->WeaponRowName;
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
