// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/SpaceCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include <sstream>

// Sets default values
ASpaceCharacter::ASpaceCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; 
	CameraBoom->bUsePawnControlRotation = true; 

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;
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
		if (FlyAction)
		{
			EnhancedInput->BindAction(FlyAction, ETriggerEvent::Started, this, &ASpaceCharacter::ToggleFlyingMode);
		}
	}
}

// Called when the game starts or when spawned
void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
}

// Called every frame
void ASpaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFlyingMode)
	{
		ConsumeFuel(DeltaTime);
	}
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
		const FRotator YawRotation(0, ControlRotation.Yaw, 0);

		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

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
	/*
	std::stringstream ss("Jump Started\n");
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, ss.str().c_str());
	*/
	Jump();
}

void ASpaceCharacter::StopJump()
{
	StopJumping();
}

void ASpaceCharacter::StartAim()
{
	/*
	std::stringstream ss("Aim Started\n");
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, ss.str().c_str());
	*/
	bIsAiming = true;
}

void ASpaceCharacter::StopAim()
{
	/*
	std::stringstream ss("Aim Stop\n");
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, ss.str().c_str());
	*/
	bIsAiming = false;
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

