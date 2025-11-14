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
#include <SpaceCharacter/States/S_Idle.h>
#include <SpaceCharacter/States/S_Aim.h>
#include <SpaceCharacter/States/S_Charging.h>

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

		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASpaceCharacter::StartSprint);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASpaceCharacter::StopSprint);
		}

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
		if (FireAction)
		{
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ASpaceCharacter::OnFireStarted);
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Completed, this, &ASpaceCharacter::OnFireCompleted);
		}

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

void ASpaceCharacter::ChangeState(ECharacterState NewState)
{
	if (CurrentState == NewState)
		return;

	if (CurrentStateObject)
		CurrentStateObject->Exit(this);

	CurrentState = NewState;

	if (StateMap.Contains(NewState))
	{
		CurrentStateObject = StateMap[NewState];
		if (CurrentStateObject)
			CurrentStateObject->Enter(this);
	}
}

void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (StateMap.Num() == 0)
	{
		StateMap.Add(ECharacterState::Locomotion, NewObject<US_Idle>(this));
		StateMap.Add(ECharacterState::Aiming, NewObject<US_Aim>(this));
		if (ChargingStateClass) {
			StateMap.Add(ECharacterState::Charging, NewObject<US_Charging>(this, ChargingStateClass));
		}

	}

	// 기본 상태 설정
	ChangeState(ECharacterState::Locomotion);
	TargetSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void ASpaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentStateObject)
		CurrentStateObject->Tick(this, DeltaTime);

	if (bIsCameraTransitioning)
		UpdateCameraTransition(DeltaTime);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		float CurrentSpeed = Move->MaxWalkSpeed;
		float NewSpeed = FMath::FInterpTo(CurrentSpeed, TargetSpeed, DeltaTime, SprintInterpSpeed);
		Move->MaxWalkSpeed = NewSpeed;
	}

	if (bIsFlyingMode)
		ConsumeFuel(DeltaTime);
	else
		RechargeFuel(DeltaTime);
}

void ASpaceCharacter::StartSprint()
{
	bIsSprinting = true;
	TargetSpeed = RunSpeed; // 목표 속도만 설정 (즉시 변경 X)
	SpawnEffectArray(SprintEffect, ActiveSprintEffects);
}

void ASpaceCharacter::StopSprint()
{
	bIsSprinting = false;
	TargetSpeed = WalkSpeed; // 감속 목표 설정
	StopEffectArray(ActiveSprintEffects);
}

void ASpaceCharacter::SpawnEffectArray(UParticleSystem* Effect, TArray<UParticleSystemComponent*>& ActiveArray)
{
	if (!Effect || !GetMesh()) return;

	// 기존 효과 제거
	StopEffectArray(ActiveArray);

	// 날개 좌우에 부착
	ActiveArray.Add(
		UGameplayStatics::SpawnEmitterAttached(
			Effect,
			GetMesh(),
			TEXT("Wing_L"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		)
	);

	ActiveArray.Add(
		UGameplayStatics::SpawnEmitterAttached(
			Effect,
			GetMesh(),
			TEXT("Wing_R"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		)
	);
}

void ASpaceCharacter::StopEffectArray(TArray<UParticleSystemComponent*>& ActiveArray)
{
	for (auto* Comp : ActiveArray)
	{
		if (Comp)
			Comp->DeactivateSystem();
	}
	ActiveArray.Empty();
}

void ASpaceCharacter::OnFireStarted(const FInputActionInstance& /*Instance*/)
{
	if (CurrentState != ECharacterState::Aiming)
		return;

	// [수정] 즉시 상태를 바꾸는 대신, ChargeStartDelay 이후에 StartCharge 함수를
	// 실행하도록 타이머를 설정합니다.
	GetWorldTimerManager().SetTimer(
		ChargeDelayHandle,
		this,
		&ASpaceCharacter::StartCharge,
		ChargeStartDelay,
		false
	);
}

void ASpaceCharacter::OnFireCompleted(const FInputActionInstance& /*Instance*/)
{
	// 1. "탭 발사" (버튼을 0.2초 안에 뗌)
	// 만약 ChargeDelayHandle 타이머가 여전히 활성화 상태라면 (즉, StartCharge가 호출되기 전)
	if (GetWorldTimerManager().IsTimerActive(ChargeDelayHandle))
	{
		// 타이머를 취소해서 Charge 상태로 들어가지 않도록 합니다.
		GetWorldTimerManager().ClearTimer(ChargeDelayHandle);

		// 여기가 "탭 발사" 로직입니다.
		// (산탄총, 단발권총 등)
		if (Shooter && CurrentState == ECharacterState::Aiming)
		{
			Shooter->SetFireDirection(FollowCamera->GetForwardVector());
			Shooter->TryFire();
			PlayFireMontage(); // 필요시 발사 몽타주 재생
		}
	}
	// 2. "차지 발사" (버튼을 0.2초 이상 누르다 뗌)
	// 만약 현재 상태가 Charging이라면 (즉, StartCharge가 이미 호출됨)
	else if (CurrentState == ECharacterState::Charging)
	{
		// Aiming 상태로 복귀시킵니다.
		// 이 호출이 S_Charging::Exit_Implementation을 트리거하여
		// 충전된 발사체를 발사하게 됩니다.
		ChangeState(ECharacterState::Aiming);
		PlayFireMontage();

	}
}

void ASpaceCharacter::SetState(ECharacterState NewState)
{
	if (CurrentState == NewState) return;

	switch (NewState)
	{
	case ECharacterState::Aiming:
		bIsAiming = true;
		break;
	case ECharacterState::Flying:
		bIsFlyingMode = true;
		break;
	default:
		bIsAiming = false;
		bIsFlyingMode = false;
		break;
	}

	CurrentState = NewState;
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
	if (bIsFlyingMode)
	{
		const float UpLaunchPower = 2000.f;

		LaunchCharacter(FVector::UpVector * UpLaunchPower, false, false);

		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			if (FlyUpMontage)
				Anim->Montage_Play(FlyUpMontage);
		}
		GetWorldTimerManager().SetTimer(
			FlightDelayHandle,
			this,
			&ASpaceCharacter::ActivateFlyingMode,
			0.5f,
			false
		);

		return;
	}

	Jump();
}

void ASpaceCharacter::StopJump()
{
	StopJumping();

	if (bIsFlyingMode)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Stop(0.1f, FlyUpMontage);
		}
	}
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
	ChangeState(ECharacterState::Aiming);
}

void ASpaceCharacter::StopAim()
{
	ChangeState(ECharacterState::Locomotion);
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
	Move->MaxFlySpeed = 150000.f;
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

void ASpaceCharacter::StartCharge()
{
	ChangeState(ECharacterState::Charging);
}

void ASpaceCharacter::PlayFireMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (FireMontage && !AnimInstance->Montage_IsPlaying(FireMontage))
		{
			UE_LOG(LogTemp, Log, TEXT("ASpaceCharacter::PlayFireMontage: --- Playing FireMontage! ---"));
			AnimInstance->Montage_Play(FireMontage);
		}
	}
}

void ASpaceCharacter::Boost()
{
	if (CurrentFuel < BoostFuelCost || bIsAiming)
		return;

	if (!bIsFlyingMode)
		ActivateFlyingMode();


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