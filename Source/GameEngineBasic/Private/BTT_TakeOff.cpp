// BTT_TakeOff.cpp
#include "BTT_TakeOff.h"
#include "EnemyHuman.h"
#include "AIController.h"

UBTT_TakeOff::UBTT_TakeOff()
{
	NodeName = "Take Off (Jump Up)";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_TakeOff::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Elapsed = 0.f;

	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return EBTNodeResult::Failed;

	AEnemyHuman* Enemy = Cast<AEnemyHuman>(AI->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	Enemy->StartTakeOff(UpSpeed, Duration);

	return EBTNodeResult::InProgress;
}

void UBTT_TakeOff::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Elapsed += DeltaSeconds;
	if (Elapsed >= Duration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
