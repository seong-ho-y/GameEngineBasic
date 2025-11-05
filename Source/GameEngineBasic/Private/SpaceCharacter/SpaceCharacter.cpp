// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/SpaceCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"

#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

#include <sstream>

ASpaceCharacter::ASpaceCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("DefaultBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; 
	CameraBoom->bUsePawnControlRotation = true; 
	CameraBoom->SocketOffset = FVector::ZeroVector;

	DefaultArmLength = CameraBoom->TargetArmLength;
	DefaultSocketOffset = CameraBoom->SocketOffset;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DefaultCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	Shooter = CreateDefaultSubobject<UShooterComp>(TEXT("ShooterComp"));
}

void ASpaceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceCharacter::Move);
		if (LookAction)
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceCharacter::Look);
		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &ASpaceCharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &ASpaceCharacter::StopJump);
		}
		if (AimAction)
		{
			EnhancedInput->BindAction(AimAction, ETriggerEvent::Started, this, &ASpaceCharacter::StartAim);
			EnhancedInput->BindAction(AimAction, ETriggerEvent::Completed, this, &ASpaceCharacter::StopAim);
		}

		EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ASpaceCharacter::StartCharge);
		EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &ASpaceCharacter::ReleaseCharge);

		if (FlyAction)
		{
			EnhancedInput->BindAction(FlyAction, ETriggerEvent::Started, this, &ASpaceCharacter::ToggleFlyingMode);
		}
		if (BoostAction)
		{
			EnhancedInput->BindAction(BoostAction, ETriggerEvent::Started, this, &ASpaceCharacter::Boost);
		}
	}
}

void ASpaceCharacter::SetChargeLevel(int32 NewLevel)
{

}

void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	bIsAiming = false;
}

void ASpaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCameraTransitioning)
		UpdateCameraTransition(DeltaTime);

	if (bIsFlyingMode)
		ConsumeFuel(DeltaTime);
	else
		RechargeFuel(DeltaTime);
}

void ASpaceCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveValue = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		FVector ForwardDir, RightDir;

		if (bIsFlyingMode)
		{
			ForwardDir = ControlRotation.Vector(); 
			RightDir = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);
		}
		else
		{
			const FRotator YawRotation(0, ControlRotation.Yaw, 0);
			ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		}
		AddMovementInput(ForwardDir, MoveValue.Y);
		AddMovementInput(RightDir, MoveValue.X);
	}
}

void ASpaceCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxis = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxis.X);
	AddControllerPitchInput(-LookAxis.Y);
}

void ASpaceCharacter::StartJump()
{
	Jump();
}

void ASpaceCharacter::StopJump()
{
	StopJumping();
}

void ASpaceCharacter::StartCharge()
{
	if (!bIsAiming || bIsCharging) return;

	bIsCharging = true;
	ChargeStartTime = GetWorld()->GetTimeSeconds();
	CurrentChargeTime = 0.f;

	FVector MuzzleLoc = GetMesh()->GetSocketLocation(TEXT("Muzzle"));
	FRotator MuzzleRot = GetMesh()->GetSocketRotation(TEXT("Muzzle"));

	if (ChargingEffect)
	{
		ActiveChargeEffect = UGameplayStatics::SpawnEmitterAttached(
			ChargingEffect,
			GetMesh(),
			TEXT("Muzzle"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	GetWorldTimerManager().SetTimer(
		ChargeTickHandle,
		this,
		&ASpaceCharacter::UpdateChargeTime,
		0.05f,
		true
	);
}

void ASpaceCharacter::UpdateChargeTime()
{
	CurrentChargeTime = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	const float ChargeRatio = FMath::Clamp(CurrentChargeTime / MaxChargeTime, 0.f, 1.f);

	if (ActiveChargeEffect)
	{
		const float Scale = FMath::Lerp(0.5f, 3.0f, ChargeRatio);
		ActiveChargeEffect->SetWorldScale3D(FVector(Scale));
	}
}

void ASpaceCharacter::ReleaseCharge()
{
	if (!bIsCharging) return;
	bIsCharging = false;
	GetWorldTimerManager().ClearTimer(ChargeTickHandle);

	if (ActiveChargeEffect)
	{
		ActiveChargeEffect->DeactivateSystem();
		ActiveChargeEffect = nullptr;
	}

	const float Elapsed = GetWorld()->GetTimeSeconds() - ChargeStartTime;
	const float ClampedCharge = FMath::Clamp(Elapsed, 0.f, MaxChargeTime);

	if (ClampedCharge < 0.2f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, TEXT("Not enough charge!"));
		return;
	}

	const float ChargeRatio = ClampedCharge / MaxChargeTime;
	Shooter->PendingDamage = FMath::Lerp(5.f, 50.f, ChargeRatio);
	Shooter->PendingScale = FMath::Lerp(1.f, 3.f, ChargeRatio);


	// 정확한 조준 방향 계산 (카메라 기준)
	FVector CameraLoc = FollowCamera->GetComponentLocation();
	FVector CameraDir = FollowCamera->GetForwardVector();
	FVector TraceEnd = CameraLoc + (CameraDir * 10000.f);

	FHitResult HitResult;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLoc, TraceEnd, ECC_Visibility))
	{
		TraceEnd = HitResult.ImpactPoint;
	}

	FVector MuzzleLoc = GetMesh()->GetSocketLocation(TEXT("Muzzle"));
	FVector FireDir = (TraceEnd - MuzzleLoc).GetSafeNormal();
	FRotator CamRot = FollowCamera->GetComponentRotation();
	SetActorRotation(FRotator(0.f, CamRot.Yaw, 0.f));

	Shooter->ProjectileClass = BaseProjectileClass;
	Shooter->SetFireDirection(FireDir);
	Shooter->TryFire();

	PlayFireMontage();
}

void ASpaceCharacter::UpdateCameraTransition(float DeltaTime)
{
	const float TargetLength = bIsAiming ? AimedArmLength : DefaultArmLength;
	const FVector TargetOffset = bIsAiming ? AimedSocketOffset : DefaultSocketOffset;

	CameraBoom->TargetArmLength = UKismetMathLibrary::FInterpTo_Constant(
	CameraBoom->TargetArmLength, TargetLength, DeltaTime, CameraInterpSpeed);

	CameraBoom->SocketOffset = UKismetMathLibrary::VInterpTo_Constant(
	CameraBoom->SocketOffset, TargetOffset, DeltaTime, CameraInterpSpeed);

	if (FMath::IsNearlyEqual(CameraBoom->TargetArmLength, TargetLength, 0.1f) &&
		CameraBoom->SocketOffset.Equals(TargetOffset, 0.1f))
	{
	CameraBoom->TargetArmLength = TargetLength;
	CameraBoom->SocketOffset = TargetOffset;
	bIsCameraTransitioning = false;
	}
}

void ASpaceCharacter::StartAim()
{
	bIsAiming = true;
	bIsCameraTransitioning = true;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	bUseControllerRotationYaw = true;
}

void ASpaceCharacter::StopAim()
{
	bIsAiming = false;
	bIsCameraTransitioning = true;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;
}

void ASpaceCharacter::ToggleFlyingMode()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Try to fly!"));
	UCharacterMovementComponent* Move = GetCharacterMovement();

	if (bIsFlyingMode)
	{
		bIsFlyingMode = false;
		Move->SetMovementMode(MOVE_Walking);
		Move->GravityScale = 1.0f;            
		Move->BrakingFrictionFactor = 2.0f;   
		Move->AirControl = 0.2f;
		Move->MaxWalkSpeed = 600.f;
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (FlypreMontage)
			AnimInstance->Montage_Play(FlypreMontage);
	}

	if (CurrentFuel > 5.f)
	{
		if (!GetWorldTimerManager().IsTimerActive(FlightDelayHandle))
		{
			GetWorldTimerManager().SetTimer(
				FlightDelayHandle,
				this,
				&ASpaceCharacter::ActivateFlyingMode,
				1.0f,
				false
			);
		}
	}
}

void ASpaceCharacter::ActivateFlyingMode()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("fly!"));

	if (CurrentFuel <= 5.f)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Not enough fuel to fly!"));
		return;
	}

	UCharacterMovementComponent* Move = GetCharacterMovement();
	bIsFlyingMode = true;

	Move->SetMovementMode(MOVE_Flying);
	Move->GravityScale = 0.05f;
	Move->BrakingFrictionFactor = 0.0f;
	Move->AirControl = 1.0f;
	Move->MaxFlySpeed = 1500.f;

}

void ASpaceCharacter::ConsumeFuel(float DeltaTime)
{
	CurrentFuel = FMath::Max(0.f, CurrentFuel - FuelConsumeRate * DeltaTime);
	
	std::stringstream ss;
	ss << "Current Fuel: " << CurrentFuel << "\n";
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, ss.str().c_str());
	if (CurrentFuel <= 0.f)
	{
		bIsFlyingMode = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

void ASpaceCharacter::RechargeFuel(float DeltaTime)
{
	CurrentFuel = FMath::Min(MaxFuel, CurrentFuel + FuelRechargeRate * DeltaTime);
}

void ASpaceCharacter::PlayFireMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (FireMontage)
		{
			AnimInstance->Montage_Play(FireMontage);
		}
	}
}

void ASpaceCharacter::Boost()
{
	if (CurrentFuel < BoostFuelCost || !bIsFlyingMode)
		return;

	
	FVector InputDir = GetLastMovementInputVector();
	FVector BoostDir = InputDir.GetSafeNormal();
	bIsBoosting = true;

	

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->AddImpulse(BoostDir * BoostStrength, true);
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (BoostMontage)
			AnimInstance->Montage_Play(BoostMontage);
	}

	GetWorldTimerManager().SetTimer(BoostHandle, this, &ASpaceCharacter::EndBoost, BoostDuration, false);
	CurrentFuel -= BoostFuelCost;
}

void ASpaceCharacter::EndBoost()
{
	bIsBoosting = false;
}