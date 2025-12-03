#include "BTT_DashAttack.h"
#include "EnemyHuman.h"
#include "AIController.h"

UBTT_DashAttack::UBTT_DashAttack()
{
	NodeName = "Dash Attack";
	bNotifyTick = false;
}

EBTNodeResult::Type UBTT_DashAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	CachedOwnerComp = &OwnerComp;

	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI) return EBTNodeResult::Failed;

	AEnemyHuman* Enemy = Cast<AEnemyHuman>(AI->GetPawn());
	if (!Enemy) return EBTNodeResult::Failed;

	// 1) 기 모으기 애니메이션 재생
	Enemy->PlayDashChargeMontage();

	// 대시 시간 (초) — 직접 원하는 값을 넣으면 됨
	const float DashDuration = 2.f;

	// 타이머 세팅
	Enemy->GetWorld()->GetTimerManager().SetTimer(
		DashTimerHandle,
		FTimerDelegate::CreateUObject(this, &UBTT_DashAttack::OnDashFinished),
		DashDuration,
		false
	);

	return EBTNodeResult::InProgress;
}

void UBTT_DashAttack::OnDashFinished()
{
	if (!CachedOwnerComp) return;

	AAIController* AI = CachedOwnerComp->GetAIOwner();
	if (!AI)
		return;

	AEnemyHuman* Enemy = Cast<AEnemyHuman>(AI->GetPawn());
	if (!Enemy)
		return;

	// ===========================
	// 1) LeftBlade 정리
	// ===========================
	if (Enemy->LeftBlade)
	{
		Enemy->LeftBlade->DeactivateHitbox();
		Enemy->LeftBlade->SetActorHiddenInGame(true);
	}

	// ===========================
	// 2) Dash 종료 처리
	// ===========================
	Enemy->EndDash();

	// ===========================
	// 3) Timer 정리
	// ===========================
	Enemy->GetWorldTimerManager().ClearTimer(DashTimerHandle);

	// ===========================
	// 4) BTTask 종료
	// ===========================
	FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
}