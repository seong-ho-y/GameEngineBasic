// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"

#include "EnemyHuman.h"
#include "BehaviorTree/BlackboardComponent.h"

AEnemyAIController::AEnemyAIController()
{
	BB = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BB"));
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	CachedEnemy = Cast<AEnemyHuman>(InPawn);
	if (!CachedEnemy.IsValid()) return;

	// BT/BB 초기화
	if (UBehaviorTree* BT = CachedEnemy->BehaviorTreeAsset)
	{
		UseBlackboard(BT->BlackboardAsset, BB);
		BB->SetValueAsBool(TEXT("HasLOS"), false);
		BB->SetValueAsFloat(TEXT("LastSeenTime"), -10000.f);
		BB->SetValueAsInt(TEXT("State"), 0); // Idle
		BB->SetValueAsFloat(TEXT("FireRange"), /*ShooterComp에서 정의해줄거임*/ 1800.f);
		RunBehaviorTree(BT);
	}

	// PawnSensing 이벤트 바인딩
	if (UPawnSensingComponent* Sensing = CachedEnemy->PawnSensingComp)
	{
		Sensing->OnSeePawn.RemoveAll(this);   // 중복 바인딩 방지
		Sensing->OnSeePawn.AddDynamic(this, &ThisClass::OnSeePawn);
	}
}

void AEnemyAIController::OnSeePawn(APawn* SeenPawn)
{
	BB->SetValueAsObject(TEXT("TargetActor"), SeenPawn);
	BB->SetValueAsVector(TEXT("LastKnownPos"), SeenPawn->GetActorLocation());
	BB->SetValueAsBool(TEXT("HasLOS"), true);
	BB->SetValueAsFloat(TEXT("LastSeenTime"), GetWorld()->GetTimeSeconds());
	BB->SetValueAsInt(TEXT("State"), 2); // Combat

	SetFocus(SeenPawn); // 전투 진입 시 시선 고정(원한다면)
}
