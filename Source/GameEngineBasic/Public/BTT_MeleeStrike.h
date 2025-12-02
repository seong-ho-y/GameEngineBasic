// ---------------------------
// BTT_MeleeStrike.h
// ---------------------------

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_MeleeStrike.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UBTT_MeleeStrike : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_MeleeStrike();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	UPROPERTY()
	class AEnemyHuman* CachedEnemy;

	float Elapsed = 0.f;
	float Timeout = 5.f; // 안전장치
};
