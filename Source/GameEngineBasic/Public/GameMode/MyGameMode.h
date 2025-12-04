// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyPlayerController.h"
#include "MyGameMode.generated.h"


UCLASS()
class GAMEENGINEBASIC_API AMyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AMyGameMode();

protected:
	virtual void BeginPlay() override;

public:

	UPROPERTY(EditAnywhere, Category = "Respawn")
	AActor* DefaultSpawnPoint;


	UFUNCTION(BlueprintCallable)
	void RespawnPlayer(AController* Controller);

	UFUNCTION(BlueprintCallable)
	void RequestStageTransition(FName TargetStageName);
};
