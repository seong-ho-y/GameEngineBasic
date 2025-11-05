// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/BTT_Shoot.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"

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

	USkeletalMeshComponent* Mesh = Self->FindComponentByClass<USkeletalMeshComponent>();
	FVector MuzzleLocation = Mesh ? Mesh->GetSocketLocation(MuzzleName) : Self->GetActorLocation();

	FVector Direction = (Target->GetActorLocation() - MuzzleLocation).GetSafeNormal();
	ShooterComp->SetFireDirection(Direction);
	UE_LOG(LogTemp, Warning, TEXT("Muzzle: %s, Target: %s"), *MuzzleLocation.ToString(), *Target->GetActorLocation().ToString());

	//발사 로직 작성
	const bool bOk = ShooterComp->TryFire();
	if (!bOk)
	{
		return EBTNodeResult::Failed;
	}
	return EBTNodeResult::Succeeded;
}
