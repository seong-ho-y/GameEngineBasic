// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_QuickBoostDash.h"
#include "AIController.h"
#include "EnemyHuman.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"

UBTT_QuickBoostDash::UBTT_QuickBoostDash()
{
	NodeName = TEXT("Quick Boost Dash");
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_QuickBoostDash::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* Char = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;
	if (!Char) return EBTNodeResult::Failed;
	
	AEnemyHuman* Enemy = AICon ? Cast<AEnemyHuman>(AICon->GetPawn()) : nullptr;
	UCharacterMovementComponent* Move = Char->GetCharacterMovement();
	if (!Move) return EBTNodeResult::Failed;

	if (bUseTargetDirection)
	{
		AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor"));
		if (!Target) return EBTNodeResult::Failed;
		BoostDir = (Target->GetActorLocation() - Char->GetActorLocation()).GetSafeNormal2D();
	}
	else
	{
		BoostDir = Char->GetActorForwardVector();
	}

	Enemy->StartBoost(BoostDir, BoostSpeed, BoostDuration, GlideDecelRate, GravityScaleDuringBoost);
	return EBTNodeResult::Succeeded;
}
void UBTT_QuickBoostDash::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* Char = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;
	if (!Char) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

	UCharacterMovementComponent* Move = Char->GetCharacterMovement();
	Timer += DeltaSeconds;
	// Phase 1: Boost 유지
	if (Timer < BoostDuration)
	{
		Move->Velocity = BoostDir * BoostSpeed;
		return;
	}

	// Phase 2: Glide → 감속
	Move->Velocity = FMath::VInterpTo(Move->Velocity, FVector::ZeroVector, DeltaSeconds, GlideDecelRate);

	// 종료 조건
	if (Move->Velocity.SizeSquared2D() < 10.f)
	{
		Move->GravityScale = OriginalGravity;
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTT_QuickBoostDash::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	ACharacter* Char = AICon ? Cast<ACharacter>(AICon->GetPawn()) : nullptr;
	if (Char)
	{
		Char->GetCharacterMovement()->GravityScale = OriginalGravity;
	}
	return EBTNodeResult::Aborted;
}
