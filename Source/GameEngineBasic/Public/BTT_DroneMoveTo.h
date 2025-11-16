#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_DroneMoveTo.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UBTT_DroneMoveTo : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_DroneMoveTo();

protected:
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;

	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;

	virtual uint16 GetInstanceMemorySize() const override
	{
		return sizeof(FDroneMoveToMemory);
	}

public:
	// Target 블랙보드 키
	UPROPERTY(EditAnywhere, Category="Blackboard")
	struct FBlackboardKeySelector TargetKey;

	// 도착 허용 거리
	UPROPERTY(EditAnywhere, Category="Settings")
	float StopDistance = 300.f;

private:
	// Task 상태 저장용 구조체
	struct FDroneMoveToMemory
	{
		FVector TargetLocation;
		bool bInitialized = false;
	};
};
