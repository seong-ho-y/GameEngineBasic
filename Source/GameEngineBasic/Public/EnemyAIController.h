// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyAIController.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AEnemyAIController();
protected:
	virtual void OnPossess(APawn* InPawn) override;

	virtual void Tick(float DeltaSeconds) override;
	
private:
	UPROPERTY()
	UBlackboardComponent* BB;

	UFUNCTION()
	void OnSeePawn(APawn* SeenPawn);
	void OnLostSight() const;

	TWeakObjectPtr<class AEnemyHuman> CachedEnemy;
};
