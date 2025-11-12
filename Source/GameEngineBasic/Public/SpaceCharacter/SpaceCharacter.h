// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceCharacter/States/CharacterStateBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "SpaceCharacter.generated.h"

class UShooterComp;
class AProjectile;

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Idle,
	Walking,
	Aiming,
	Charging,
	Flying,
	Boosting,
	Jumping
};

UCLASS()
class GAMEENGINEBASIC_API ASpaceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceCharacter();

protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "State")
	TMap<ECharacterState, UCharacterStateBase*> StateMap;

	UPROPERTY(VisibleInstanceOnly, Category = "State")
	UCharacterStateBase* CurrentStateObject = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ECharacterState CurrentState = ECharacterState::Idle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* FollowCamera;

	// Component
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UShooterComp* Shooter;

	// 입력
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FlyAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BoostAction;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* FlypreMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* BoostMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* FlyUpMontage;

public:
	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float RunSpeed = 750.f;

	// Fuel
	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float MaxFuel = 100.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float CurrentFuel = 100.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float FuelConsumeRate = 12.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float FuelRechargeRate = 6.f;          

	// Boost
	UPROPERTY(EditAnywhere, Category = "Flight|Boost")
	float BoostStrength = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Boost")
	float BoostDuration = 2.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Boost")
	float BoostCooldown = 5.f;

	// Projectile
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile>  BaseProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Charge")
	float MaxChargeTime = 6.0f; 

	UPROPERTY(EditAnywhere, Category = "Charge")
	UParticleSystem* ChargingEffect;

	UPROPERTY()
	UParticleSystemComponent* ActiveChargeEffect;

protected:
	void ChangeState(ECharacterState NewState);
	
	virtual void BeginPlay() override;
	void Boost();
	void EndBoost();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartJump();
	void StopJump();

	void StartAim();
	void StopAim();
	void UpdateCameraTransition(float DeltaTime);

	void StartCharge();
	void ReleaseCharge();
	void UpdateChargeTime();

	void PlayFireMontage();

	void ActivateFlyingMode();
	void ToggleFlyingMode();
	void ConsumeFuel(float DeltaTime);
	void RechargeFuel(float DeltaTime);
	

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetState(ECharacterState NewState);

	void StartSprint();
	void StopSprint();

	ECharacterState GetCurrentState() const { return CurrentState; }

public:
	bool bIsBoosting = false;
	bool bIsAiming = false;
	bool bIsSprinting = false;

public:
	UFUNCTION(BlueprintCallable, Category = "Charge")
	void SetChargeLevel(int32 NewLevel);

private:
	FTimerHandle BoostHandle;
	FTimerHandle FlightDelayHandle;
	FTimerHandle ChargeTickHandle;

	// 비행 관련 변수
	float BoostFuelCost = 20.f;
	bool bIsFlyingMode = false;

	// 카메라 기본 거리
	float DefaultArmLength = 300.f;
	float AimedArmLength = 180.f;

	// Aim 시 카메라 위치 오프셋
	FVector DefaultSocketOffset = FVector::ZeroVector;
	FVector AimedSocketOffset = FVector(0.f, 60.f, -20.f);

	// 카메라 변수
	float CameraInterpSpeed = 1000.f;
	bool bIsCameraTransitioning = false;

	// Charge 변수
	bool bIsCharging = false;
	float ChargeStartTime = 0.f;
	float CurrentChargeTime = 0.f;
};
