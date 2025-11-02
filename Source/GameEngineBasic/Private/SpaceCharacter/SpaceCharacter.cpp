// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/SpaceCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include <sstream>

// Sets default values
ASpaceCharacter::ASpaceCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
		if(FireAction)
		{
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Triggered, this, &ASpaceCharacter::FireTriggered);
			EnhancedInput->BindAction(FireAction, ETriggerEvent::Started, this, &ASpaceCharacter::FireStarted);
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

// Called when the game starts or when spawned
void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	bIsAiming = false;
}

// Called every frame
void ASpaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsCameraTransitioning)
		UpdateCameraTransition(DeltaTime);

	if (bIsFlyingMode)
		ConsumeFuel(DeltaTime);
	else
	if (GetCharacterMovement()->IsFalling() && GetCharacterMovement()->MovementMode == MOVE_Walking)
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
			// 비행 중일 때는 Pitch도 포함
			ForwardDir = ControlRotation.Vector(); // 전체 회전 방향
			RightDir = FRotationMatrix(ControlRotation).GetUnitAxis(EAxis::Y);
		}
		else
		{
			// 걷기 모드에서는 평면 기준 이동
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

void ASpaceCharacter::FireStarted(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("FireStarted"));
	if (bIsAiming && Shooter->TryFire()) {
		PlayFireMontage(); 
	}
}

void ASpaceCharacter::FireTriggered(const FInputActionValue& Value)
{
	UE_LOG(LogTemp, Warning, TEXT("FireTriggered"));
	if (bIsAiming && Shooter->TryFire()) {
		PlayFireMontage();
	}
}

void ASpaceCharacter::UpdateCameraTransition(float DeltaTime)
{
	
}

void ASpaceCharacter::StartAim()
{
	bIsAiming = true;
	bIsCameraTransitioning = true;
}

void ASpaceCharacter::StopAim()
{
	bIsAiming = false;
	bIsCameraTransitioning = true;
}

void ASpaceCharacter::ToggleFlyingMode()
{
	// 이미 비행 중이면 해제
	if (bIsFlyingMode)
	{
		bIsFlyingMode = false;
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		return;
	}
	
	// 비행 시작: 연료가 일정량 이상 있어야 함
	if (CurrentFuel > 5.f)
	{
		bIsFlyingMode = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);

		auto* Move = GetCharacterMovement();
		Move->BrakingFrictionFactor = 0.2f;
		Move->MaxFlySpeed = 900.f; 
	}
}

void ASpaceCharacter::ConsumeFuel(float DeltaTime)
{
	CurrentFuel = FMath::Max(0.f, CurrentFuel - FuelConsumeRate * DeltaTime);
	
	std::stringstream ss;
	ss << "Current Fuel: " << CurrentFuel << "\n";
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Yellow, ss.str().c_str());
	// 연료 고갈 시 즉시 비행 해제
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

void ASpaceCharacter::Boost(const FInputActionValue& Value)
{
	if (!bIsFlyingMode || !bCanBoost || CurrentFuel < BoostFuelCost)
		return;

	const FVector2D InputValue = Value.Get<FVector2D>();
	const FRotator ControlRot = Controller->GetControlRotation();

	FVector BoostDir = FVector::ZeroVector;
	const FVector ForwardDir = UKismetMathLibrary::GetForwardVector(ControlRot);
	const FVector RightDir = UKismetMathLibrary::GetRightVector(ControlRot);

	// 방향 입력이 없으면 전방 Boost
	if (InputValue.IsNearlyZero())
	{
		BoostDir = ForwardDir;
	}
	else
	{
		BoostDir = (ForwardDir * InputValue.Y + RightDir * InputValue.X).GetSafeNormal();
	}

	// Boost 조건 설정
	bIsBoosting = true;
	bCanBoost = false;
	CurrentFuel = FMath::Max(0.f, CurrentFuel - BoostFuelCost);

	// 순간 가속 (짧은 Dash 느낌)
	LaunchCharacter(BoostDir * BoostStrength, true, true);

	// Boost 종료 및 감속 처리
	GetWorldTimerManager().SetTimer(BoostHandle, [this]()
		{
			bIsBoosting = false;
			FVector CurrentVel = GetCharacterMovement()->Velocity;
			GetCharacterMovement()->Velocity = CurrentVel * 0.4f; // 감속
		}, BoostDuration, false);

	// Boost 쿨타임 시작
	FTimerHandle CooldownHandle;
	GetWorldTimerManager().SetTimer(CooldownHandle, [this]()
		{
			bCanBoost = true;
		}, BoostCooldown, false);

}