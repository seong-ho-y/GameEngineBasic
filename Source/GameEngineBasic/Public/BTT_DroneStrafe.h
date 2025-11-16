#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_DroneStrafe.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UBTT_DroneStrafe : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_DroneStrafe();

	// 지속 시간
	UPROPERTY(EditAnywhere, Category="Drone")
	float StrafeDuration = 0.4f;

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
		return sizeof(FStrafeMemory);
	}

private:
	struct FStrafeMemory
	{
		float Elapsed = 0.f;
		bool bIsRight = false;
	};
};
