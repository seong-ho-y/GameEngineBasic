// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_Random.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UBTS_Random : public UBTService
{
	GENERATED_BODY()

public:
	UBTS_Random();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

public:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	struct FBlackboardKeySelector RandomKey;

	// 최소/최대 random 값
	UPROPERTY(EditAnywhere, Category="Random")
	float MinValue = 0.f;

	UPROPERTY(EditAnywhere, Category="Random")
	float MaxValue = 100.f;

	// 랜덤 갱신 간격
	UPROPERTY(EditAnywhere, Category="Random")
	float UpdateInterval = 1.0f;

private:
	float TimeSinceLastUpdate = 0.f;
};
