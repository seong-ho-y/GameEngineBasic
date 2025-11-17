#include "BTT_DroneRetreat.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyDrone.h"
#include "DroneMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UBTT_DroneRetreat::UBTT_DroneRetreat()
{
	NodeName = "Drone Retreat";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_DroneRetreat::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyDrone* Drone = AI ? Cast<AEnemyDrone>(AI->GetPawn()) : nullptr;

	if (!Drone || !Drone->DroneMoveComp)
		return EBTNodeResult::Failed;

	// 초기 시작에서는 그냥 InProgress
	return EBTNodeResult::InProgress;
}

void UBTT_DroneRetreat::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyDrone* Drone = AI ? Cast<AEnemyDrone>(AI->GetPawn()) : nullptr;

	if (!Drone || !Drone->DroneMoveComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AActor* Player = UGameplayStatics::GetPlayerPawn(Drone, 0);
	if (!Player)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector DroneLoc = Drone->GetActorLocation();
	FVector PlayerLoc = Player->GetActorLocation();

	float Dist = FVector::Dist2D(DroneLoc, PlayerLoc);

	// 목표 거리까지 멀어졌으면 끝
	if (Dist >= StopDistance)
	{
		Drone->DroneMoveComp->Velocity = FVector::ZeroVector;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 아직 가까우면 뒤로 이동
	Drone->DroneMoveComp->MoveAway(PlayerLoc);
}
