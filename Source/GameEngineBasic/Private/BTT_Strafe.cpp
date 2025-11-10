// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Strafe.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTT_Strafe::UBTT_Strafe()
{
	NodeName = "Combat Strafe";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_Strafe::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* Char = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;
	if (!Char) return EBTNodeResult::Failed;

	if (Move = Char->GetCharacterMovement(); !Move) return EBTNodeResult::Failed;

	const int32 DirSign = FMath::RandBool() ? 1 : -1;
	FVector RightVec = Char->GetActorRightVector();
	StrafeDir = RightVec * DirSign;

	ElapsedTime = 0.f;
	Duration = FMath::RandRange(1.0f, 2.0f);
	StrafeSpeed = 600.f;

	return EBTNodeResult::InProgress;
}

void UBTT_Strafe::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	if (!Move) {FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return;}
	ElapsedTime += DeltaSeconds;

	FVector Vel = StrafeDir * StrafeSpeed;
	Move->Velocity = Vel;

	if (ElapsedTime >= Duration)
	{
		Move->StopMovementImmediately();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
