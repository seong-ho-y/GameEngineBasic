// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "DroneMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UDroneMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UDroneMovementComponent();

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Hover")
	float HoverHeight = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Hover")
	float HoverInterpSpeed = 5.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Hover")
	float HoverTraceDistance = 1500.f;
	
	void MaintainHover(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Movement")
	float MoveSpeed = 600.f;

	void MoveInDirection(const FVector& WorldDirection);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Movement")
	float StrafeSpeed = 900.f;

	void StrafeLeft();
	void StrafeRight();

	void MoveToward(const FVector& Target);
	void MoveAway(const FVector& Target);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Movement")
	float VelocityDamp = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Movement")
	bool bHovering = true;
	
protected:
	void ApplyDamping(float DeltaTime);

private:
	
	
};
