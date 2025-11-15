// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyHuman.h"
#include "Components/ActorComponent.h"
#include "TargetingSystemComponent.generated.h"

class UImage;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTargetChanged, AEnemyHuman*, NewTarget);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetingSystemComponent();

	// 🔹 크로스헤어 원의 반지름(픽셀 단위)
	UPROPERTY(EditAnywhere, Category="Targeting|UI")
	float TargetCircleRadius = 200.f;
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AEnemyHuman* GetCurrentTarget() const  {return CurrentTarget; }

	UPROPERTY(BlueprintAssignable, Category = "Targeting")
	FOnTargetChanged OnTargetChanged;

private:
	AEnemyHuman* FindClosestEnemyInView();

	UPROPERTY()
	UImage* TargetCircleWidget;
	
	UPROPERTY()
	AEnemyHuman* CurrentTarget = nullptr; 

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float ViewAngle = 30.f;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float MaxDistance = 8000.f;
};
