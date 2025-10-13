// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/BTT_SetThrottle.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTT_SetThrottle::UBTT_SetThrottle()
{
	NodeName = TEXT("Set Throttle");
}

EBTNodeResult::Type UBTT_SetThrottle::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}
	BB->SetValueAsFloat(ThrottleBlackboardKey.SelectedKeyName, ThrottleValueToSet);

	return EBTNodeResult::Succeeded;
}
