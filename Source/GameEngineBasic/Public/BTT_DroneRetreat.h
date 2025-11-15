#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_DroneRetreat.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UBTT_DroneRetreat : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_DroneRetreat();

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
	

	UPROPERTY(EditAnywhere, Category="Drone")
	float StopDistance = 700.f; // 멈춤 허용 거리
};
