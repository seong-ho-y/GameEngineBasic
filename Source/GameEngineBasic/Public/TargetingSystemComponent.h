// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyHuman.h"
#include "Components/ActorComponent.h"
#include "TargetingSystemComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UTargetingSystemComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetingSystemComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AEnemyHuman* GetCurrentTarget() const  {return CurrentTarget; }

private:
	AEnemyHuman* FindClosestEnemyInView();

	UPROPERTY()
	AEnemyHuman* CurrentTarget; 

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float ViewAngle = 30.f;

	UPROPERTY(EditAnywhere, Category = "Targeting")
	float MaxDistance = 8000.f;
};
