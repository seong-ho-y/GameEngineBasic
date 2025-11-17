// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TurretAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UTurretAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret")
	FRotator HeadAimRotation;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
