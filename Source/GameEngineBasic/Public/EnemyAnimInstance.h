// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Movement")
	bool bIsInAir;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Combat")
	bool bIsAttacking;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	bool bIsGroggy;
};
