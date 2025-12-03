#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_DashAttack.generated.h"

UCLASS()
class UBTT_DashAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_DashAttack();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


	UFUNCTION()
	void OnDashFinished();
private:	
	UPROPERTY()
	UBehaviorTreeComponent* CachedOwnerComp;
	FTimerHandle DashTimerHandle;
};
