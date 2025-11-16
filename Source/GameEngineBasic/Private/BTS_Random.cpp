// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_Random.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTS_Random::UBTS_Random()
{
	NodeName = "Make Random Num";
	bNotifyBecomeRelevant = true;
	bNotifyTick = true;
}
void UBTS_Random::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!OwnerComp.GetAIOwner()) return;
	TimeSinceLastUpdate += DeltaSeconds;
	if (TimeSinceLastUpdate >= UpdateInterval)
	{
		TimeSinceLastUpdate = 0.f;

		float Value = FMath::FRandRange(MinValue, MaxValue);

		if (UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent())
			BB->SetValueAsFloat(RandomKey.SelectedKeyName, Value);
	}
}
