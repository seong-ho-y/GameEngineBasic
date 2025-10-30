// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "SpaceCharacter.generated.h"

class UShooterComp;

UCLASS()
class GAMEENGINEBASIC_API ASpaceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceCharacter();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

	// Component
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UShooterComp* Shooter;

	// ют╥б
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FlyAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;

	// Fuel
	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float MaxFuel = 100.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float CurrentFuel = 100.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float FuelConsumeRate = 12.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float FuelRechargeRate = 6.f;
protected:
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartJump();
	void StopJump();

	void StartAim();
	void StopAim();

	void ToggleFlyingMode();
	void ConsumeFuel(float DeltaTime);
	void RechargeFuel(float DeltaTime);
public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void FireTriggered(const FInputActionValue& Value); 
	void FireStarted(const FInputActionValue& Value);
private:
	bool bIsAiming = false;
	bool bIsFlyingMode = false;

	
};
