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
class UHealthComp;
class UShieldComp;
class UFuelComponent;
class UWingComponent;

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
	FlyAim,
	FlyCharge
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
	ECharacterState CurrentState = ECharacterState::Locomotion;

	// Component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UShooterComp* Shooter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFuelComponent* Fuel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWingComponent* WingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComp* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShieldComp* ShieldComp;

	// Input
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* BoostAction;

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
	UInputAction* ShieldAction;

public:
	FORCEINLINE class UShooterComp* GetShooterComponent() const { return Shooter; }

	FORCEINLINE class UFuelComponent* GetFuelComponent() const { return Fuel; }

	FORCEINLINE class UCameraComponent* GetFollowCameraComponent() const { return FollowCamera; }

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UWingComponent* GetWingComponent() const { return WingComp; }

	FORCEINLINE class UHealthComp* GetHealthComponent() const { return HealthComp; }

	FORCEINLINE class UShieldComp* GetShieldComponent() const { return ShieldComp; }

public:
	// Particle System Components
	UPROPERTY()
	UParticleSystemComponent* ActiveChargeEffect;

	// Anim Montages
	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* FireMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* FlyUpMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* BoostMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* ShieldMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* HitMontage;

	// Particle Systems
	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* ChargingEffect;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* ShieldEffect;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* DeathExplosionEffect;

	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float RunSpeed = 1200.f;

	// 사망
	UPROPERTY(EditAnywhere, Category = "State|Movement")
	float RagdollDuration = 3.0f; 

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

public:
	// Shield
	UFUNCTION()
	void OnShieldActivated();

	UFUNCTION()
	void OnShieldDeactivated();

	UFUNCTION()
	void OnShieldKeyPressed(const FInputActionInstance& Instance);

	// Die
	UFUNCTION()
	void OnCharacterDeath(AActor* DeadActor);

	void ExplodeAndDestroy();

public:
	virtual void BeginPlay() override;

	void HandleSprintOrBoostInput(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void ToggleFlyingMode();

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

	void StartCharge();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	void ChangeState(ECharacterState NewState);
	ECharacterState GetCurrentState() const { return CurrentState; }
protected:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetState(ECharacterState NewState);


public:
	bool bIsBoosting = false;
	bool bIsAiming = false;
	bool bIsSprinting = false;
	bool bIsFlyingMode = false;

	float ChargeStartTime = 0.f;
	float TargetSpeed = 0.f;

public:
	FTimerHandle BoostHandle;
	FTimerHandle FlightDelayHandle;
	FTimerHandle ChargeDelayHandle; 
	FTimerHandle DeathTimerHandle;

	// 카메라 기본 거리
	float DefaultArmLength = 300.f;
	float AimedArmLength = 180.f;

	// Aim 시 카메라 위치 오프셋
	FVector DefaultSocketOffset = FVector::ZeroVector;
	FVector AimedSocketOffset = FVector(0.f, 60.f, -20.f);

	// 카메라 변수
	float CameraInterpSpeed = 1000.f;
	bool bIsCameraTransitioning = false;

	float SprintInterpSpeed = 5.f;

	// 사망 여부
	bool bIsDead = false;
};
