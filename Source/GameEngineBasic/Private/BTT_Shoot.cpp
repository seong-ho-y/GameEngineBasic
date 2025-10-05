// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Shoot.h"

#include "AIController.h"
#include "ShooterComp.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_Shoot::UBTT_Shoot()
{
	NodeName = TEXT("Shoot");
	bCreateNodeInstance = false;
	
}

EBTNodeResult::Type UBTT_Shoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	APawn* Self = AI ? AI->GetPawn() : nullptr;
	if (!Self) return EBTNodeResult::Failed;

	//컴포넌트 서치
	auto* ShooterComp = Self->FindComponentByClass<UShooterComp>();
	if (!ShooterComp)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (!Target) return EBTNodeResult::Failed;

	//발사 로직 작성
	const bool bOk = ShooterComp->TryFire();
	if (!bOk)
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}
