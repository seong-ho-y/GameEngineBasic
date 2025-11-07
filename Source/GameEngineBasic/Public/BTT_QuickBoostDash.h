// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_QuickBoostDash.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UBTT_QuickBoostDash : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_QuickBoostDash();

	UPROPERTY(EditAnywhere, Category = "Boost")
	bool bUseTargetDirection = true;

	UPROPERTY(EditAnywhere, Category = "Boost")
	float BoostSpeed = 30000.f;

	UPROPERTY(EditAnywhere, Category = "Boost")
	float BoostDuration = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Boost")
	float GlideDecelRate = 6.0f;

	UPROPERTY(EditAnywhere, Category = "Boost")
	float GravityScaleDuringBoost = 0.4;


protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


private:
	float Timer = 0.f;
	FVector BoostDir = FVector::ZeroVector;
	float OriginalGravity = 1.f;
	
};
