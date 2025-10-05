// Fill out your copyright notice in the Description page of Project Settings.


#include "BTS_Rotate.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnitConversion.h"

UBTS_Rotate::UBTS_Rotate()
{
	NodeName = TEXT("Rotate");
	bNotifyTick = true;
	Interval = 0.0f;
	RandomDeviation = 0.0f;
}
void UBTS_Rotate::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return;
	APawn* Self = AI ? AI->GetPawn() : nullptr;
	if (!Self) return;
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetKeySelector.SelectedKeyName));

	if (Target)
	{
		FVector AILoc = Self->GetActorLocation();
		FVector TargetLocation = Target->GetActorLocation();
		FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(AILoc, TargetLocation);
		TargetRotation.Roll = 0.f;

		FRotator NewRotation = FMath::RInterpTo(
			Self->GetActorRotation(),
			TargetRotation,
			DeltaSeconds,
			RotationSpeed);
		
		Self->SetActorRotation(NewRotation);
	}
}
