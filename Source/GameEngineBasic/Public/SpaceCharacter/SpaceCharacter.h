// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceCharacter/States/CharacterStateBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "TargetingSystemComponent.h"
#include "SpaceCharacter.generated.h"

class UShooterComp;
class AProjectile;
class US_Charging;

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	Locomotion,
	Aiming,
	Charging,
	Flying,
	Boosting,
	Jumping
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnergyChanged, float, CurrentEN, float, MaxEN);

UCLASS()
class GAMEENGINEBASIC_API ASpaceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASpaceCharacter();

	UPROPERTY(BlueprintAssignable)
	FOnEnergyChanged OnEnergyChanged;
protected:
	UPROPERTY(EditAnywhere, Instanced, Category = "State")
	TMap<ECharacterState, UCharacterStateBase*> StateMap;

	UPROPERTY(VisibleInstanceOnly, Category = "State")
	UCharacterStateBase* CurrentStateObject = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	ECharacterState CurrentState = ECharacterState::Locomotion;

	// --- Sprint�� ---
	UPROPERTY(EditAnywhere, Category = "VFX|Sprint")
	UParticleSystem* SprintEffect;

	UPROPERTY(VisibleAnywhere, Category = "VFX|Sprint")
	TArray<UParticleSystemComponent*> ActiveSprintEffects;

	UPROPERTY(EditAnywhere, Category = "VFX|Sprint")
	float SprintInterpSpeed = 5.0f;

	// --- Flying�� ---
	UPROPERTY(EditAnywhere, Category = "VFX|Flying")
	UParticleSystem* FlyingEffect;

	UPROPERTY(VisibleAnywhere, Category = "VFX|Flying")
	TArray<UParticleSystemComponent*> ActiveFlyingEffects;

	// Component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UShooterComp* Shooter;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UTargetingSystemComponent* TargetingSys;

	// Input
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
	FORCEINLINE class UShooterComp* GetShooterComponent() const { return Shooter; }

	FORCEINLINE class UCameraComponent* GetFollowCameraComponent() const { return FollowCamera; }

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

public:
	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float RunSpeed = 750.f;

	

	// Fuel
	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float MaxEN = 100.f;

	UPROPERTY(EditAnywhere, Category = "Flight|Fuel")
	float CurrentEN = 100.f;

	UFUNCTION(BlueprintCallable, Category = "Flight|Fuel")
	float GetFuelPercent() const
	{
		return (MaxEN > 0) ? CurrentEN / MaxEN : 0.f;
	}

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

	// Charge
	UPROPERTY(EditAnywhere, Category = "State")
	TSubclassOf<US_Charging> ChargingStateClass;

	UPROPERTY(EditAnywhere, Category = "Charge")
	float ChargeStartDelay = 0.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge")
	bool bIsCharging = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Charge")
	float CurrentChargeTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "Charge")
	float MaxChargeTime = 6.0f;

	UPROPERTY(EditAnywhere, Category = "Charge")
	UParticleSystem* ChargingEffect;

	UPROPERTY()
	UParticleSystemComponent* ActiveChargeEffect;

public:

	void SpawnEffectArray(UParticleSystem* Effect, TArray<UParticleSystemComponent*>& ActiveArray);
	void StopEffectArray(TArray<UParticleSystemComponent*>& ActiveArray);


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

	void OnFireStarted(const struct FInputActionInstance& Instance);
	void OnFireCompleted(const struct FInputActionInstance& Instance);
	void PlayFireMontage();

	void ActivateFlyingMode();
	void ToggleFlyingMode();
	void ConsumeFuel(float DeltaTime);
	void RechargeFuel(float DeltaTime);


	void StartCharge();

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetState(ECharacterState NewState);
	void ChangeState(ECharacterState NewState);

	void StartSprint();
	void StopSprint();

	ECharacterState GetCurrentState() const { return CurrentState; }

public:
	bool bIsBoosting = false;
	bool bIsAiming = false;
	bool bIsSprinting = false;
	float ChargeStartTime = 0.f;
	float TargetSpeed = 0.f;
public:
	FTimerHandle BoostHandle;
	FTimerHandle FlightDelayHandle;
	FTimerHandle ChargeDelayHandle;

	// ���� ���� ����
	float BoostFuelCost = 20.f;
	bool bIsFlyingMode = false;

	// ī�޶� �⺻ �Ÿ�
	float DefaultArmLength = 300.f;
	float AimedArmLength = 180.f;

	// Aim �� ī�޶� ��ġ ������
	FVector DefaultSocketOffset = FVector::ZeroVector;
	FVector AimedSocketOffset = FVector(0.f, 60.f, -20.f);

	// ī�޶� ����
	float CameraInterpSpeed = 1000.f;
	bool bIsCameraTransitioning = false;

	// Add Return Func for UI
};
