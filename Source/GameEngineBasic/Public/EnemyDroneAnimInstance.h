// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyDroneAnimInstance.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EDroneState : uint8
{
	Idle,
	Move,
	Hit,
	Death,
	OpenClaw,
	CloseClaw
};

UCLASS()
class GAMEENGINEBASIC_API UEnemyDroneAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Speed;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Direction;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EDroneState DroneState = EDroneState::Idle;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bPlayWingAdditive = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double MoveDirForward;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double MoveDirRight;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	APawn* OwnerPawn;
};
