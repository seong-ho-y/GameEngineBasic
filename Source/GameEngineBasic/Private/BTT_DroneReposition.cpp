#include "BTT_DroneReposition.h"
#include "AIController.h"
#include "EnemyDrone.h"
#include "DroneMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UBTT_DroneReposition::UBTT_DroneReposition()
{
	NodeName = "Drone Reposition";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_DroneReposition::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AEnemyDrone* Drone = AI ? Cast<AEnemyDrone>(AI->GetPawn()) : nullptr;

	if (!Drone || !Drone->DroneMoveComp)
		return EBTNodeResult::Failed;

	FRepositionMemory* Mem = (FRepositionMemory*)NodeMemory;

	// 플레이어 기준 랜덤 위치 생성
	AActor* Player = UGameplayStatics::GetPlayerPawn(Drone, 0);
	if (!Player)
		return EBTNodeResult::Failed;

	FVector PlayerLoc = Player->GetActorLocation();

	// 랜덤 반경 생성
	float Radius = FMath::RandRange(MinRadius, MaxRadius);
	float Angle = FMath::RandRange(0.f, 360.f);

	// Offset 계산 (XZ 평면)
	FVector Offset(
		FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
		FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
		0.f
	);

	Mem->TargetLocation = PlayerLoc + Offset;
	Mem->bInitialized = true;

	return EBTNodeResult::InProgress;
}

void UBTT_DroneReposition::TickTask(
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

	FRepositionMemory* Mem = (FRepositionMemory*)NodeMemory;
	if (!Mem->bInitialized)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 이동
	Drone->DroneMoveComp->MoveToward(Mem->TargetLocation);

	float Dist = FVector::Dist2D(
		Drone->GetActorLocation(),
		Mem->TargetLocation
	);

	// 가까워지면 끝
	if (Dist <= AcceptRadius)
	{
		Drone->DroneMoveComp->Velocity = FVector::ZeroVector;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
