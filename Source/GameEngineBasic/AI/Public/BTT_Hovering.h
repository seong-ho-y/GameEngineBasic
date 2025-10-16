// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Hovering.generated.h"

class UNiagaraSystem;

struct FBT_Hovering_Memory 
{
	// 이동할 목표 지점을 저장합니다.
	FVector TargetLocation;
};

UCLASS()
class GAMEENGINEBASIC_API UBTT_Hovering : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_Hovering();

protected:
	// 태스크 시작 시 호출
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

public:
	// 에디터에서 설정할 변수들
	UPROPERTY(EditAnywhere, Category = "AI Hover")
	float HoverRadius = 2000.0f; // 현재 위치에서 얼마나 먼 곳까지 갈지 결정

	UPROPERTY(EditAnywhere, Category = "VFX")
	UNiagaraSystem* WarpVFX;
};