// BTT_ShortDash.cpp
#include "BTT_ShortDash.h"
#include "EnemyHuman.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_ShortDash::UBTT_ShortDash()
{
	NodeName = "Short Dash";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_ShortDash::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Elapsed = 0.f;

	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return EBTNodeResult::Failed;

	AEnemyHuman* Enemy = Cast<AEnemyHuman>(AI->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB) return EBTNodeResult::Failed;

	AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!Target)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShortDash: TargetActor is NULL"));
		return EBTNodeResult::Failed;
	}

	// ================================
	//   ★ Target 기준 로컬 방향 → 월드 방향
	//   DashDirection : (X = Forward, Y = Right) 로 해석
	// ================================
	const FVector TargetForward = Target->GetActorForwardVector();
	const FVector TargetRight   = Target->GetActorRightVector();

	// Z는 무시하고 XY 평면만 사용
	FVector LocalDir = DashDirection;
	LocalDir.Z = 0.f;

	FVector WorldDir =
		TargetForward * LocalDir.X +
		TargetRight   * LocalDir.Y;

	WorldDir.Z = 0.f;
	WorldDir   = WorldDir.GetSafeNormal();

	if (!WorldDir.IsNearlyZero())
	{
		Enemy->PlayDashEffects(WorldDir);
		Enemy->StartShortDash(WorldDir, DashSpeed, DashDuration);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ShortDash: WorldDir is zero, using Target->Forward"));
		FVector Fallback = TargetForward.GetSafeNormal2D();
    
		// ★ Dash Effects 실행
		Enemy->PlayDashEffects(Fallback);
		Enemy->StartShortDash(TargetForward.GetSafeNormal2D(), DashSpeed, DashDuration);
	}

	return EBTNodeResult::InProgress;
}
void UBTT_ShortDash::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Elapsed += DeltaSeconds;

	if (Elapsed >= DashDuration)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTT_ShortDash::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type Result)
{
	// 특별히 정리할 건 없음
}
