// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExecutionComp.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecutionStart, AActor*, TargetEnemy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnExecutionEnd, AActor*, TargetEnemy);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UExecutionComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UExecutionComp();

protected:
	virtual void BeginPlay() override;

public:
	// ====== 처형 기능 공개 API ======
	UFUNCTION(BlueprintCallable, Category = "Execution")
	bool StartExecution();

	UFUNCTION(BlueprintCallable, Category = "Execution")
	void EndExecution();

	// ====== Ray 기반 타겟 탐색 ======
	UFUNCTION(BlueprintCallable, Category = "Execution")
	AActor* FindExecutionTarget() const;

public:
	// ====== 브로드캐스트 ======
	UPROPERTY(BlueprintAssignable, Category = "Execution|Event")
	FOnExecutionStart OnExecutionStart;

	UPROPERTY(BlueprintAssignable, Category = "Execution|Event")
	FOnExecutionEnd OnExecutionEnd;
		
protected:
	UPROPERTY(EditAnywhere, Category = "Execution")
	float ExecutionRange = 600.f;

	// Enemy의 Visibility Block
	UPROPERTY(EditAnywhere, Category = "Execution")
	TEnumAsByte<ECollisionChannel> ExecutionTraceChannel = ECC_Visibility;
	
	UPROPERTY(EditAnywhere, Category = "Execution")
	bool bDebugDraw = false;
	
};
