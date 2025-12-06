// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveSystem.generated.h"


UCLASS()
class GAMEENGINEBASIC_API USaveSystem : public USaveGame
{
	GENERATED_BODY()
	
public:
    // ----------- Location -----------
    UPROPERTY(BlueprintReadWrite)
    FVector SavedLocation;

    UPROPERTY(BlueprintReadWrite)
    FRotator SavedRotation;

    // ----------- Stat -----------
    UPROPERTY(BlueprintReadWrite)
    float CurrentHealth;

    UPROPERTY(BlueprintReadWrite)
    float MaxHealth;

    UPROPERTY(BlueprintReadWrite)
    float CurrentShield;

    UPROPERTY(BlueprintReadWrite)
    float MaxShield;

    UPROPERTY(BlueprintReadWrite)
    float CurrentFuel;

    UPROPERTY(BlueprintReadWrite)
    float MaxFuel;

    // ----------- Ability -----------
    UPROPERTY(BlueprintReadWrite)
    bool bCanSprint;

    UPROPERTY(BlueprintReadWrite)
    bool bCanDash;

    UPROPERTY(BlueprintReadWrite)
    bool bCanFly;

    UPROPERTY(BlueprintReadWrite)
    bool bCanShield;

    // ----------- Weapon -----------
    UPROPERTY(BlueprintReadWrite)
    TArray<FName> WeaponRowName;

    UPROPERTY(BlueprintReadWrite)
    int32 CurrentAmmo;

    UPROPERTY(BlueprintReadWrite)
    int32 FullAmmo;

    UPROPERTY(BlueprintReadWrite)
    int32 MaxAmmo;
};
