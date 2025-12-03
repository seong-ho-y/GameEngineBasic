// BTT_TakeOff.h
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_TakeOff.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UBTT_TakeOff : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_TakeOff();

	UPROPERTY(EditAnywhere, Category="TakeOff")
	float UpSpeed = 1200.f;

	UPROPERTY(EditAnywhere, Category="TakeOff")
	float Duration = 0.4f;

	float Elapsed = 0.f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
