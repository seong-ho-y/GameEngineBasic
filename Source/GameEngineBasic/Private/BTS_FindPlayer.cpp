// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_FindPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTS_FindPlayer::UBTS_FindPlayer()
{
	NodeName = "FindPlayer";
	bNotifyTick = true;
	Interval = 0.5f;
	RandomDeviation = 0.0f;
}

void UBTS_FindPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AI = OwnerComp.GetAIOwner();
	APawn* Self = AI ? AI->GetPawn() : nullptr;
	if (!Self) return;

	AActor* Target = Self->GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!Target) return;
	const float Distance = FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation());
	if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
	{
		BB->SetValueAsObject(TargetActorKey.SelectedKeyName, Target);
		BB->SetValueAsVector(TargetLocKey.SelectedKeyName, Target->GetActorLocation());
		BB->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, Distance);
	}
} 

