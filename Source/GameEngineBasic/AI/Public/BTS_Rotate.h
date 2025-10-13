// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_Rotate.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UBTS_Rotate : public UBTService
{
	GENERATED_BODY()
public:
	UBTS_Rotate();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlackBoard")
	FBlackboardKeySelector TargetKeySelector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BlackBoard")
	float RotationSpeed = 5.0f;

	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
