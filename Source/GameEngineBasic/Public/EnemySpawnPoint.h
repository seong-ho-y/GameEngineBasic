// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameEngineBasic/Enemy/Public/EnemyPoolManager.h"
#include "GameFramework/Actor.h"
#include "EnemySpawnPoint.generated.h"

UCLASS()
class GAMEENGINEBASIC_API AEnemySpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemySpawnPoint();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	bool bUseCustomProfile = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn", meta=(EditCondition="bUseCustomProfile"))
	FEnemySpawnProfile CustomSpawnProfile;

	bool HasCustomProfile() const {return bUseCustomProfile; }
	const FEnemySpawnProfile& GetCustomProfile() const {return CustomSpawnProfile; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
