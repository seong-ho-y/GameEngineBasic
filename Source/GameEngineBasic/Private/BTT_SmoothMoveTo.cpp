// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_SmoothMoveTo.h"

#include "AIController.h"
#include "AI/NavigationModifier.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/MovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"

class UFloatingPawnMovement;

UBTT_SmoothMoveTo::UBTT_SmoothMoveTo()
{
	NodeName = TEXT("Smooth MoveTo");
	bNotifyTick = true;           // 매 틱 갱신
	bCreateNodeInstance = true;   // 노드 인스턴스 유지 (상태 보관 용이)
}

EBTNodeResult::Type UBTT_SmoothMoveTo::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	if (!AICon || !Pawn)
	{
		UE_LOG(LogTemp,Error,TEXT("[SmoothMoveTo] No AICon or Pawn :: ExecuteTask"));
		return EBTNodeResult::Failed;
	}

	// 목표 확인
	FVector TargetLoc;
	if (!GetTargetLocation(OwnerComp, TargetLoc))
	{
		UE_LOG(LogTemp,Error,TEXT("[SmoothMoveTo] No Target :: ExecuteTask"));
		return bFailIfNoTarget ? EBTNodeResult::Failed : EBTNodeResult::Succeeded;
	}

	// 시작 시점엔 그대로 진행 (Tick에서 처리)
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTT_SmoothMoveTo::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 필요 시 감속 마무리 없이 즉시 중단
	return EBTNodeResult::Aborted;
}

void UBTT_SmoothMoveTo::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AICon = OwnerComp.GetAIOwner();
	APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
	if (!AICon || !Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FVector TargetLoc;
	if (!GetTargetLocation(OwnerComp, TargetLoc))
	{
		FinishLatentTask(OwnerComp, bFailIfNoTarget ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
		return;
	}

	const FVector CurrLoc = Pawn->GetActorLocation();

	// Calculate Direction, Distance (you can calculate for 3D using by bUse3DDirection)
	FVector Dir;
	float Distance = 0.f;
	ComputeDirectionDistance(CurrLoc, TargetLoc, Dir, Distance);

	// Current Velocity
	const UMovementComponent* MoveComp = Pawn->GetMovementComponent();
	const FVector CurrVel = MoveComp ? MoveComp->Velocity : FVector::ZeroVector;
	const float Speed = bUse3DDirection ? CurrVel.Size() : CurrVel.Size2D();

	// Get Goal Velocity
	const float DesiredSpeed = ComputeDesiredSpeed(Distance, Speed);

	// Accel CurrVel to Goal Vel
	const float NewSpeed = ApplyAcceleration(Speed, DesiredSpeed, DeltaSeconds);

	// Calculate Throttle to use AddMovementInput
	const float Throttle = (MaxSpeed > KINDA_SMALL_NUMBER) ? FMath::Clamp(NewSpeed / MaxSpeed, 0.f, 1.f) : 0.f;

	// if Dir=0 -> Don't give any MovementInput
	if (!Dir.IsNearlyZero())
	{
		Pawn->AddMovementInput(Dir, Throttle);
	}

	// Process Rotation
	if (bRotateToVelocity && !Dir.IsNearlyZero())
	{
		const FRotator CurrRot = Pawn->GetActorRotation();
		const FRotator TargetRot = Dir.Rotation();
		const FRotator NewRot = FMath::RInterpTo(CurrRot, TargetRot, DeltaSeconds, YawInterpSpeed);
		Pawn->SetActorRotation(NewRot);
	}

	// Check get to targetLoc
	if (Distance <= AcceptanceRadius && Speed <= StopSpeedThreshold)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
}

bool UBTT_SmoothMoveTo::GetTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const
{
	const UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return false;

	// 1) Actor 우선
	if (TargetActorKey.SelectedKeyType)
	{
		UObject* Obj = BB->GetValueAsObject(TargetActorKey.SelectedKeyName);
		if (const AActor* TargetActor = Cast<AActor>(Obj))
		{
			OutLocation = TargetActor->GetActorLocation();
			return true;
		}
	}

	// 2) Location 사용
	if (TargetLocationKey.SelectedKeyType)
	{
		const FVector Loc = BB->GetValueAsVector(TargetLocationKey.SelectedKeyName);
		if (Loc.IsNearlyZero() == false)
		{
			OutLocation = Loc;
			return true;
		}
	}

	return false;
}

float UBTT_SmoothMoveTo::ApplyAcceleration(float CurrentSpeed, float DesiredSpeed, float DeltaSeconds) const
{
	const float Rate = (DesiredSpeed > CurrentSpeed) ? Accel : Decel; // 가/감속 분리
	return FMath::FInterpConstantTo(CurrentSpeed, DesiredSpeed, DeltaSeconds, Rate);
}

float UBTT_SmoothMoveTo::ComputeDesiredSpeed(float DistanceToTarget, float CurrentSpeed) const
{
	// 브레이크 거리: s = v^2 / (2a)
	const float SafeDecel = FMath::Max(Decel, 1.f);
	const float BrakeDist = (CurrentSpeed * CurrentSpeed) / (2.f * SafeDecel);

	// 도착 반경을 제외한 실거리
	const float SafeDist = FMath::Max(DistanceToTarget - AcceptanceRadius, 0.f);

	if (SafeDist <= BrakeDist)
	{
		// 감속구간: 거리 d에서 정지 가능하도록 목표속도 v = sqrt(2 a d)
		return FMath::Sqrt(2.f * SafeDecel * SafeDist);
	}
	else
	{
		// 가속/순항구간
		return MaxSpeed;
	}
}

void UBTT_SmoothMoveTo::ComputeDirectionDistance(const FVector& From, const FVector& To, FVector& OutDir, float& OutDistance) const
{
	const FVector Delta = To - From;

	if (bUse3DDirection)
	{
		OutDistance = Delta.Size();
		OutDir = OutDistance > KINDA_SMALL_NUMBER ? Delta.GetSafeNormal() : FVector::ZeroVector;
	}
	else
	{
		const FVector Delta2D(Delta.X, Delta.Y, 0.f);
		OutDistance = Delta2D.Size();
		OutDir = OutDistance > KINDA_SMALL_NUMBER ? Delta2D.GetSafeNormal() : FVector::ZeroVector;
	}
}