// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceCharacter/States/CharacterStateBase.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"
#include "InputAction.h"
#include "PlayerStatsComponent.h"
#include "TargetingSystemComponent.h"
#include "WeaponComponent.h"
#include "SpaceCharacter.generated.h"

class UShooterComp;
class UHealthComp;
class UShieldComp;
class UFuelComponent;
class UWingComponent;
class UExecutionComp;

class AProjectile;
class AAblityUnlockItem;
class US_Charging;
class IU_Interactable;
class AMyPlayerState;

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

public:
	// Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPlayerStatsComponent* StatsComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShooterComp* Shooter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWeaponComponent* WeaponComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UFuelComponent* Fuel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWingComponent* WingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComp* HealthComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShieldComp* ShieldComp;

	// Sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Utility")
	USoundBase* DashSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Utility")
	USoundBase* JumpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound|Utility")
	USoundBase* DeathSound;


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UTargetingSystemComponent* TargetingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UExecutionComp* ExecutionComp;

	// 🔹 PlayerState에서 스탯+무기 적용하는 헬퍼
	UFUNCTION()
	void InitFromPlayerState();

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
	UInputAction* DashAction;

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

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ReloadAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SwapWeaponAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ExecuteAction;

	UPROPERTY(EditAnywhere, Category = "Execution|VFX")
	UParticleSystem* ExecutionTeleportVFX;

public:
	FORCEINLINE class UShooterComp* GetShooterComponent() const { return Shooter; }

	FORCEINLINE class UFuelComponent* GetFuelComponent() const { return Fuel; }

	FORCEINLINE class UCameraComponent* GetFollowCameraComponent() const { return FollowCamera; }

	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UWingComponent* GetWingComponent() const { return WingComp; }

	FORCEINLINE class UHealthComp* GetHealthComponent() const { return HealthComp; }

	FORCEINLINE class UShieldComp* GetShieldComponent() const { return ShieldComp; }

	FORCEINLINE class UExecutionComp* GetExecutionComponent() const { return ExecutionComp; }

	FORCEINLINE class UWeaponComponent* GetWeaponComponent() const { return WeaponComp; }

public:
	// Interactable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TScriptInterface<IU_Interactable> CurrentInteractTarget;

	// Particle System Components
	UPROPERTY()
	UParticleSystemComponent* ActiveChargeEffect;

	// Anim Montages
	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* ChargeFireMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* SingleFireMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* FlyUpMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* BoostMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* ShieldMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* HitMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* ExecuteMontage;

	UPROPERTY(EditAnywhere, Category = "Anim")
	UAnimMontage* LevelStartMontage;

	UPROPERTY(EditAnywhere, Category = "Anim|Ability")
	TMap<EAbilityType, UAnimMontage*> AbilityUnlockMontages;

	// Particle Systems
	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* ChargingEffect;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* ShieldEffect;

	UPROPERTY(EditAnywhere, Category = "Effect")
	UParticleSystem* DeathExplosionEffect;

	// Niagara Systems
	UPROPERTY(EditDefaultsOnly, Category = "Niagara")
	UNiagaraComponent* DashVfx;

	UPROPERTY(EditDefaultsOnly, Category = "Niagara")
	UNiagaraComponent* HealVfx;

	// Movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float WalkSpeed = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State|Movement")
	float RunSpeed = 1200.f;

	// ���
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
	UFUNCTION(BlueprintCallable)
	void UnlockAbility(EAbilityType Ability);

public:
	// Shield
	UFUNCTION()
	void OnShieldActivated();

	UFUNCTION()
	void OnShieldDeactivated();

	UFUNCTION()
	void OnShieldKeyPressed(const FInputActionInstance& Instance);
	void HandleReload();

	// Die
	UFUNCTION()
	void OnCharacterDeath(AActor* DeadActor);

	void ExplodeAndDestroy();

	// Dashing Getters
	UFUNCTION(BlueprintCallable)
	bool IsDashing() const { return bIsDashing; }

public:
	virtual void BeginPlay() override;

	// Movement
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	void StartJump();
	void StopJump();

	//Sprint
	void OnSprintPressed();
	void OnSprintReleased();
	void StartSprint();
	void StopSprint();
	
	// Dash
	void StartDash();
	void StopDash();
	void StartDashEffect();
	void StopDashEffect();
	FVector GetDashDirection() const;
	uint16 DashRootMotionID = (uint16)ERootMotionSourceID::Invalid;

	// Boost
	void StartBoost();

	// Flying
	void ToggleFlyingMode();

	// Aim
	void StartAim();
	void StopAim();
	void UpdateCameraTransition(float DeltaTime);

	// Fire
	void OnFireStarted(const struct FInputActionInstance& Instance);
	void OnFireCompleted(const struct FInputActionInstance& Instance);
	void PlayChargeFireMontage();
	void PlaySingleFireMontage();

	// Charge
	void StartCharge();

	// Interact
	void TryInteract();

	// Execution
	void TryExecutionInput();
	UFUNCTION()
	void OnExecutionStart(AActor* Target);
	UFUNCTION()
	void OnExecutionEnd(AActor* Target);

	// GetDamage
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	                         class AController* EventInstigator, AActor* DamageCauser) override;

	// State
	void ChangeState(ECharacterState NewState);
	ECharacterState GetCurrentState() const { return CurrentState; }
protected:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SetState(ECharacterState NewState);

	FVector GetExecutionPosition(AActor* Target, float ForwardOffset, float UpOffset);
	void SwapWeapon();

public:
	bool bIsBoosting = false;
	bool bIsAiming = false;
	bool bIsFlyingMode = false;
	bool bSprintHeld = false;
	bool bIsSprinting = false;
	bool bIsDashing = false;

	float ChargeStartTime = 0.f;
	float TargetSpeed = 0.f;

public:
	FTimerHandle DashTimerHandle;
	FTimerHandle BoostHandle;
	FTimerHandle SprintHoldTimer;
	FTimerHandle FlightDelayHandle;
	FTimerHandle ChargeDelayHandle; 
	FTimerHandle DeathTimerHandle;
	FTimerHandle DashEffectTimerHandle;

	// Aim Length
	float DefaultArmLength = 300.f;
	float AimedArmLength = 180.f;

	// Aim Socket Offset
	FVector DefaultSocketOffset = FVector::ZeroVector;
	FVector AimedSocketOffset = FVector(0.f, 60.f, -20.f);

	// Camera
	float CameraInterpSpeed = 1000.f;
	bool bIsCameraTransitioning = false;

	float SprintInterpSpeed = 5.f;

	// Death
	bool bIsDead = false;

	UPROPERTY()
	TMap<FName, FWeaponRuntimeState> WeaponStates;
};
