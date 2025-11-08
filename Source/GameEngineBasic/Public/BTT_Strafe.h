// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Strafe.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UBTT_Strafe : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_Strafe();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	FVector StrafeDir;
	float ElapsedTime;
	float Duration;
	float StrafeSpeed;

	UPROPERTY()
	UPawnMovementComponent* Move;
};
