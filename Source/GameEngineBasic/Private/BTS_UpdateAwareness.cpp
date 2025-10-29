// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_UpdateAwareness.h"

#include "AIController.h"
#include "EnemyHuman.h"
#include "BehaviorTree/BlackboardComponent.h"

class AEnemyHuman;

UBTS_UpdateAwareness::UBTS_UpdateAwareness()
{
	NodeName = TEXT("S_UpdateAwareness");
	Interval = 0.2;
	RandomDeviation = 0.0f;
}

void UBTS_UpdateAwareness::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	auto* BB = OwnerComp.GetBlackboardComponent();
	auto* Controller = OwnerComp.GetAIOwner();
	if (!BB || !Controller) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
	APawn* Self = Controller->GetPawn();
	if (!Self) return;

	const float Now = Self->GetWorld()->GetTimeSeconds();

	bool bHasLOS = false;
	if (Target)
	{
		bHasLOS = Controller->LineOfSightTo(Target); // PawnSensing 보조: 지속 LOS 판정
		BB->SetValueAsBool(TEXT("HasLOS"), bHasLOS);
		if (bHasLOS)
		{
			BB->SetValueAsVector(TEXT("LastKnownPos"), Target->GetActorLocation());
			BB->SetValueAsFloat(TEXT("LastSeenTime"), Now);
		}
	}

	// State 갱신
	int32 NewState = 0; // Idle
	if (Target)
	{
		if (bHasLOS) NewState = 2; // Combat
		else
		{
			float Hold = 3.0f;
			if (const AEnemyHuman* EH = Cast<AEnemyHuman>(Self)) Hold = EH->AlertHoldSeconds;
			NewState = (Now - BB->GetValueAsFloat(TEXT("LastSeenTime")) <= Hold) ? 1 : 0; // Alert or Idle
		}

		// CanFire: Combat 상태 + 사거리
		const float FireRange = BB->GetValueAsFloat(TEXT("FireRange"));
		const bool bInRange = Target && FVector::Dist(Self->GetActorLocation(), Target->GetActorLocation()) <= FireRange;
		BB->SetValueAsBool(TEXT("CanFire"), (NewState == 2) && bInRange);
	}
	else
	{
		NewState = 0; // Idle
		BB->SetValueAsBool(TEXT("CanFire"), false);
	}

	BB->SetValueAsInt(TEXT("State"), NewState);
}
