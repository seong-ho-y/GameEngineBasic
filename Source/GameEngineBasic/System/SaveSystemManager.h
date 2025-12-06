// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SaveSystemManager.generated.h"


UCLASS()
class GAMEENGINEBASIC_API USaveSystemManager : public UObject
{
	GENERATED_BODY()
	
public:
    // 저장
    UFUNCTION(BlueprintCallable)
    static void SavePawnState(APawn* Pawn);

	// Ability
    UFUNCTION(BlueprintCallable)
    static void SaveAbilities(APawn* Pawn);

    // Weapon
    UFUNCTION(BlueprintCallable)
    static void SaveWeapons(APawn* Pawn);

    // 로딩 후 Pawn 데이터를 복원
    UFUNCTION(BlueprintCallable)
    static void LoadPawnState(APawn* Pawn);

    // 체크포인트 위치 저장
    UFUNCTION(BlueprintCallable)
    static void SaveSpawnPoint(const FVector& Location, const FRotator& Rotation);

    // 저장된 위치 불러오기
    UFUNCTION(BlueprintCallable)
    static bool GetSavedSpawnPoint(FVector& OutLoc, FRotator& OutRot);

};
