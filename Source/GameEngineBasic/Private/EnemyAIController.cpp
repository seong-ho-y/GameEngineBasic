// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAIController.h"

#include "EnemyAnimInstance.h"
#include "EnemyHuman.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!CachedEnemy.IsValid()) return;

	const bool bHasLOS = BB->GetValueAsBool(TEXT("HasLOS"));
	auto* Mesh = CachedEnemy->GetMesh();
	if (!Mesh) return;
	
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(Mesh->GetAnimInstance()))
	{
		if (bHasLOS)
		{
			if (APawn* Target = Cast<APawn>(BB->GetValueAsObject(TEXT("TargetActor"))))
			{
				const FVector Dir = Target->GetActorLocation() - CachedEnemy->GetActorLocation();
				const FRotator LookRot = Dir.Rotation();
				const FRotator MyRot   = CachedEnemy->GetActorRotation();

				const FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(LookRot, MyRot);

				Anim->TargetUpperYaw   = DeltaRot.Yaw;
				Anim->TargetUpperPitch = DeltaRot.Pitch;
			}
		}
		else
		{
			Anim->TargetUpperYaw   = 0.f;
			Anim->TargetUpperPitch = 0.f;
		}
	}
}

void AEnemyAIController::OnSeePawn(APawn* SeenPawn)
{
	BB->SetValueAsObject(TEXT("TargetActor"), SeenPawn);
	BB->SetValueAsVector(TEXT("TargetLocation"), SeenPawn->GetActorLocation());
	BB->SetValueAsBool(TEXT("HasLOS"), true);
	BB->SetValueAsFloat(TEXT("LastSeenTime"), GetWorld()->GetTimeSeconds());
	BB->SetValueAsInt(TEXT("State"), 2); // Combat

	SetFocalPoint(SeenPawn->GetActorLocation());
}
void AEnemyAIController::OnLostSight() const
{
	if (Blackboard)
	{
		Blackboard->ClearValue("TargetActor");
	}
}
