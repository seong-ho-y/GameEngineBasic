#include "BTT_DroneMoveTo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyDrone.h"
#include "DroneMovementComponent.h"

UBTT_DroneMoveTo::UBTT_DroneMoveTo()
{
    bNotifyTick = true;
    NodeName = TEXT("Drone MoveTo");
}

EBTNodeResult::Type UBTT_DroneMoveTo::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    AEnemyDrone* Drone = AI ? Cast<AEnemyDrone>(AI->GetPawn()) : nullptr;

    if (!Drone)
        return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;

    FDroneMoveToMemory* Mem = (FDroneMoveToMemory*)NodeMemory;

    // 1) TargetActor 우선
    UObject* TargetObj = BB->GetValueAsObject(TargetKey.SelectedKeyName);
    if (AActor* TargetActor = Cast<AActor>(TargetObj))
    {
        Mem->TargetLocation = TargetActor->GetActorLocation();
    }
    else
    {
        // 2) TargetLocation(float vector)
        Mem->TargetLocation = BB->GetValueAsVector(TargetKey.SelectedKeyName);
    }

    Mem->bInitialized = true;

    return EBTNodeResult::InProgress;
}


void UBTT_DroneMoveTo::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    AEnemyDrone* Drone = AI ? Cast<AEnemyDrone>(AI->GetPawn()) : nullptr;

    if (!Drone)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FDroneMoveToMemory* Mem = (FDroneMoveToMemory*)NodeMemory;

    if (!Mem->bInitialized)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector DroneLoc = Drone->GetActorLocation();
    float Dist = FVector::Dist2D(DroneLoc, Mem->TargetLocation);

    // 목표 도착?
    if (Dist <= StopDistance)
    {
        Drone->DroneMoveComp->Velocity = FVector::ZeroVector;
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // Target 방향으로 이동
    Drone->DroneMoveComp->MoveToward(Mem->TargetLocation);
}
