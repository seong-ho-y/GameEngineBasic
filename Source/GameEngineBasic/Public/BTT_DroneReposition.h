#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_DroneReposition.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UBTT_DroneReposition : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_DroneReposition();

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
		return sizeof(FRepositionMemory);
	}

public:
	// 랜덤 오프셋 거리
	UPROPERTY(EditAnywhere, Category="Drone")
	float MinRadius = 400.f;

	UPROPERTY(EditAnywhere, Category="Drone")
	float MaxRadius = 900.f;

	// 도착 조건
	UPROPERTY(EditAnywhere, Category="Drone")
	float AcceptRadius = 200.f;

private:
	struct FRepositionMemory
	{
		FVector TargetLocation;
		bool bInitialized = false;
	};
};
