// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SetThrottle.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UBTT_SetThrottle : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_SetThrottle();
	
protected:
	// 이 태스크가 실행될 때 호출되는 함수
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	// 비헤이비어 트리 에디터에서 직접 값을 입력할 수 있도록 노출
	UPROPERTY(EditAnywhere, Category = "AI")
	float ThrottleValueToSet = 0.0f;

	// 값을 저장할 블랙보드 키를 지정
	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector ThrottleBlackboardKey;
};
