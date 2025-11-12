// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "MyAnimInstance.generated.h"


UCLASS()
class GAMEENGINEBASIC_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    ECharacterState AnimState;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    float Speed;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    float Pitch;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    float Roll;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    float Yaw;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    float YawDelta;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    float Direction;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    bool bIsInAir;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    bool bIsAiming;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    bool bisAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    bool bIsFlying = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    bool bIsBoosting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    bool bCanBoost = true;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    FRotator MovementRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Character|Movement")
    FRotator AimRotation;

    UPROPERTY(Transient)
    class ASpaceCharacter* OwningCharacter;
public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
