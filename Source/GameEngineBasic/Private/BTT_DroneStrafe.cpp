#include "BTT_DroneStrafe.h"
#include "AIController.h"
#include "EnemyDrone.h"
#include "DroneMovementComponent.h"

UBTT_DroneStrafe::UBTT_DroneStrafe()
{
	NodeName = "Drone Strafe";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_DroneStrafe::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyDrone* Drone = AI ? Cast<AEnemyDrone>(AI->GetPawn()) : nullptr;

	if (!Drone || !Drone->DroneMoveComp)
		return EBTNodeResult::Failed;

	FStrafeMemory* Mem = (FStrafeMemory*)NodeMemory;

	// 좌우 랜덤 (-1 = Left, 1 = Right)
	Mem->bIsRight = FMath::RandBool();
	Mem->Elapsed = 0.f;

	if (Mem->bIsRight)
		Drone->DroneMoveComp->StrafeRight();
	else
		Drone->DroneMoveComp->StrafeLeft();

	return EBTNodeResult::InProgress;
}

void UBTT_DroneStrafe::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	FStrafeMemory* Mem = (FStrafeMemory*)NodeMemory;
	Mem->Elapsed += DeltaSeconds;

	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyDrone* Drone = AI ? Cast<AEnemyDrone>(AI->GetPawn()) : nullptr;

	if (!Drone || !Drone->DroneMoveComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 지속 시간 끝
	if (Mem->Elapsed >= StrafeDuration)
	{
		// 멈추기
		Drone->DroneMoveComp->Velocity = FVector::ZeroVector;

		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
