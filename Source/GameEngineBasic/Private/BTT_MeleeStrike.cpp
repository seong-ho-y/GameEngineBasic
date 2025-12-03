// ---------------------------
// BTT_MeleeStrike.cpp
// ---------------------------

#include "BTT_MeleeStrike.h"
#include "AIController.h"
#include "EnemyHuman.h"

UBTT_MeleeStrike::UBTT_MeleeStrike()
{
	NodeName = TEXT("Melee Strike Attack");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_MeleeStrike::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Elapsed = 0.f;

	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return EBTNodeResult::Failed;

	CachedEnemy = Cast<AEnemyHuman>(AI->GetPawn());
	if (!CachedEnemy) return EBTNodeResult::Failed;

	CachedEnemy->StartMeleeAttack();

	return EBTNodeResult::InProgress;
}

void UBTT_MeleeStrike::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Elapsed += DeltaSeconds;

	if (!CachedEnemy)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (CachedEnemy->IsMeleeFinished())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	if (Elapsed >= Timeout)
	{
		UE_LOG(LogTemp, Warning, TEXT("MeleeStrike timed out"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
