// BTT_ShortDash.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ShortDash.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UBTT_ShortDash : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_ShortDash();
	
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;
	
	/** Inspector에서 설정 가능 */
	UPROPERTY(EditAnywhere, Category="Dash")
	FVector DashDirection = FVector(1.f, 0.f, 0.f); // 기본 전방

	UPROPERTY(EditAnywhere, Category="Dash")
	float DashSpeed = 1500.f;

	UPROPERTY(EditAnywhere, Category="Dash")
	float DashDuration = 0.15f;

	/** 시작 시간 기록용 */
	float Elapsed = 0.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
		EBTNodeResult::Type TaskResult) override;
};
