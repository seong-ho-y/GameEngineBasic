// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "SpaceCharacter.generated.h"

UCLASS()
class GAMEENGINEBASIC_API ASpaceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceCharacter();

private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// ют╥б
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	// fire
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;

	/** Fire Input Action (Pressed/Released) */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;
protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartJump();
	void StopJump();

	void StartAim();
	void StopAim();
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	bool bIsAiming = false;
};
