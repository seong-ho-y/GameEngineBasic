// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/SpaceCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Component/FuelComponent.h"
#include "Component/WingComponent.h"
#include "Component/ShieldComp.h"
#include "Component/ExecutionComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameEngineBasic/Components/public/HealthComp.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "GameEngineBasic/Public/Item/AbilityUnlockItem.h"	
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"

#include <sstream>
#include <SpaceCharacter/States/S_Idle.h>
#include <SpaceCharacter/States/S_Aim.h>
#include <SpaceCharacter/States/S_Charging.h>
#include <SpaceCharacter/States/S_Fly.h>
#include <SpaceCharacter/States/S_Boost.h>
#include <SpaceCharacter/States/S_FlyAim.h>
#include <SpaceCharacter/States/S_FlyCharge.h>

#include "WeaponComponent.h"

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
	Fuel = CreateDefaultSubobject<UFuelComponent>(TEXT("FuelComp"));
	WingComp = CreateDefaultSubobject<UWingComponent>(TEXT("WingComp"));
	ShieldComp = CreateDefaultSubobject<UShieldComp>(TEXT("ShieldComp"));
	HealthComp = CreateDefaultSubobject<UHealthComp>(TEXT("HealthComp"));
	TargetingComp = CreateDefaultSubobject<UTargetingSystemComponent>(TEXT("TargetingComp")); 

	ExecutionComp = CreateDefaultSubobject<UExecutionComp>(TEXT("ExecutionComp"));

	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComp"));
	
}

void ASpaceCharacter::UnlockAbility(EAbilityType Ability)
{
	switch (Ability)
	{
	case EAbilityType::Sprint:
		bCanSprint = true;
		break;

	case EAbilityType::Flying:
		bCanFly = true;
		break;

	case EAbilityType::Dash:
		bCanDash = true;
		break;

	case EAbilityType::Shield:
		bCanShield = true;
		break;
	}


	// 연출
	if (UAnimMontage** MontagePtr = AbilityUnlockMontages.Find(Ability))
	{
		UAnimMontage* Montage = *MontagePtr;
		if (Montage)
		{
			if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
			{
				Anim->Montage_Play(Montage);
			}
		}
	}
}

void ASpaceCharacter::OnShieldActivated()
{
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
	{
		if (ShieldMontage && !Anim->Montage_IsPlaying(ShieldMontage))
			Anim->Montage_Play(ShieldMontage);
	}

	if (ShieldEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(
			ShieldEffect,
			GetMesh(),
			FName("Shield") 
		);
	}
}

void ASpaceCharacter::OnShieldDeactivated()
{
	// 쉴드 꺼질 때 필요한 처리
	// (예: 파티클 중지, 효과 제거 등)

	// StopAll 파티클을 사용하려면, 
	// SpawnEmitterAttached의 return 값을 저장하는 방식으로
	// “활성 파티클 포인터”를 보관하는 패턴으로 확장 가능
}

void ASpaceCharacter::OnShieldKeyPressed(const FInputActionInstance& /*Instance*/)
{
	if (ShieldComp)
		ShieldComp->ActivateShield();
}

void ASpaceCharacter::HandleReload()
{
	if (Shooter)
	{
		Shooter->StartReload();
	}
}

void ASpaceCharacter::OnCharacterDeath(AActor* DeadActor)
{
	if (bIsDead) return;
	bIsDead = true;

	if(DeathMontage)
		PlayAnimMontage(DeathMontage);

	if (Controller)
	{
		Controller->StopMovement();
		Controller->UnPossess();
	}

	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	GetWorldTimerManager().SetTimer(
		DeathTimerHandle,
		this,
		&ASpaceCharacter::ExplodeAndDestroy,
		RagdollDuration,
		false
	);
}

void ASpaceCharacter::ExplodeAndDestroy()
{
	if (DeathExplosionEffect && GetMesh())
	{
		UGameplayStatics::SpawnEmitterAttached(
			DeathExplosionEffect,
			GetMesh(),
			FName("Shield")
		);
	}

	Destroy();
}

void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	if (StateMap.Num() == 0)
	{
		StateMap.Add(ECharacterState::Locomotion, NewObject<US_Idle>(this));
		StateMap.Add(ECharacterState::Aiming, NewObject<US_Aim>(this)); 
		if (ChargingStateClass)
			StateMap.Add(ECharacterState::Charging, NewObject<US_Charging>(this, ChargingStateClass));
		StateMap.Add(ECharacterState::Flying, NewObject<US_Fly>(this));
		StateMap.Add(ECharacterState::FlyAim, NewObject<US_FlyAim>(this));
		StateMap.Add(ECharacterState::FlyCharge, NewObject<US_FlyCharge>(this));
		StateMap.Add(ECharacterState::Boosting, NewObject<US_Boost>(this));
	}

	// 기본 상태 설정
	ChangeState(ECharacterState::Locomotion);
	TargetSpeed = WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	if (ShieldComp)
	{
		ShieldComp->OnShieldActivated.AddDynamic(this, &ASpaceCharacter::OnShieldActivated);
		ShieldComp->OnShieldDeactivated.AddDynamic(this, &ASpaceCharacter::OnShieldDeactivated);
	}
	if (WingComp)
		WingComp->SetMesh(GetMesh());
	if (HealthComp)
	{
		HealthComp->OnDeath.AddDynamic(this, &ASpaceCharacter::OnCharacterDeath);
	}
	if (ExecutionComp)
	{
		ExecutionComp->OnExecutionStart.AddDynamic(this, &ASpaceCharacter::OnExecutionStart);
		ExecutionComp->OnExecutionEnd.AddDynamic(this, &ASpaceCharacter::OnExecutionEnd);
	}
	if (WeaponComp)
		WeaponComp->InitializeWeapon(this, Shooter);
}

void ASpaceCharacter::HandleSprintOrBoostInput(const FInputActionValue& Value)
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	const bool bIsInAir = Move->IsFalling();
	const bool bIsFlying = (CurrentState == ECharacterState::Flying) || bIsFlyingMode;

	// 공중이거나 비행 중일 때는 Boost 실행
	if (bIsInAir || bIsFlying)
	{
		ChangeState(ECharacterState::Boosting);
		return;
	}

	StartSprint();
}

void ASpaceCharacter::StartSprint()
{
	if (bIsBoosting) return;
	bIsSprinting = true;
	TargetSpeed = RunSpeed;
	WingComp->PlaySprint();
}

void ASpaceCharacter::StopSprint()
{
	if(bIsBoosting) return;
	bIsSprinting = false;
	TargetSpeed = WalkSpeed;
	WingComp->StopAll();
}

void ASpaceCharacter::ToggleFlyingMode()
{
	if (CurrentState == ECharacterState::Flying)
	{
		ChangeState(ECharacterState::Locomotion);
		return;
	}
	if (!Fuel || !Fuel->CanFly())
	{
		return;
	}

	ChangeState(ECharacterState::Flying);
}

void ASpaceCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveValue = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		FVector ForwardDir, RightDir;

		if (CurrentState == ECharacterState::Flying || bIsFlyingMode)
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
	if (CurrentState == ECharacterState::Flying || bIsFlyingMode)
	{
		UAnimInstance* Anim = GetMesh()->GetAnimInstance();

		if (Anim && FlyUpMontage && Anim->Montage_IsPlaying(FlyUpMontage))
			return;

		const float UpLaunchPower = 2000.f;

		LaunchCharacter(FVector::UpVector * UpLaunchPower, false, false);

		
		if (FlyUpMontage) {
			Anim->Montage_Play(FlyUpMontage);
			WingComp->PlayFly();
			return;
		}
	}

	Jump();
}

void ASpaceCharacter::StopJump()
{
	StopJumping();

	if (CurrentState == ECharacterState::Flying)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Stop(0.1f, FlyUpMontage);

			if (bIsBoosting)
				return;

			WingComp->StopAll();
		}
	}
}

void ASpaceCharacter::StartAim()
{
	bIsAiming = true;
	bIsCameraTransitioning = true;

	if (CurrentState == ECharacterState::Flying || bIsFlyingMode)
	{
		ChangeState(ECharacterState::FlyAim);
	}
	else
	{
		ChangeState(ECharacterState::Aiming);
	}
}

void ASpaceCharacter::StopAim()
{
	bIsAiming = false;
	bIsCameraTransitioning = true; // 카메라 줌 아웃을 위해 트랜지션 시작

	if (CurrentState == ECharacterState::FlyAim ||
		CurrentState == ECharacterState::FlyCharge)
	{
		ChangeState(ECharacterState::Flying);
	}
	else
	{
		ChangeState(ECharacterState::Locomotion);
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

void ASpaceCharacter::OnFireStarted(const FInputActionInstance& /*Instance*/)
{
	// 비행 
	if (CurrentState == ECharacterState::FlyAim)
	{
		ChangeState(ECharacterState::FlyCharge);
		return;
	}

	// 지상
	if (CurrentState != ECharacterState::Aiming)
		return;

	
	if (WeaponComp)
		WeaponComp->HandleFirePressed();


	//지상 Aim에서는 기존처럼 "차지 지연 타이머" 작동
	/*
	GetWorldTimerManager().SetTimer(
		ChargeDelayHandle,
		this,
		&ASpaceCharacter::StartCharge,
		ChargeStartDelay,
		false
	);
	*/
}

void ASpaceCharacter::OnFireCompleted(const FInputActionInstance& /*Instance*/)
{
	if (CurrentState == ECharacterState::FlyAim ||
		CurrentState == ECharacterState::FlyCharge)
	{
		// 비행에서는 FireCompleted가 의미 없음
		return;
	}

	
	if (WeaponComp)
		WeaponComp->HandleFireReleased();

	
	/*
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
		return;
	}
	// 만약 현재 상태가 Charging이라면 (즉, StartCharge가 이미 호출됨)
	else if (CurrentState == ECharacterState::Charging)
	{
		// Aiming 상태로 복귀시킵니다.
		// 이 호출이 S_Charging::Exit_Implementation을 트리거하여
		// 충전된 발사체를 발사하게 됩니다.
		ChangeState(ECharacterState::Aiming);
		PlayFireMontage();
		return;
	}
	*/
}

void ASpaceCharacter::PlayFireMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (FireMontage && !AnimInstance->Montage_IsPlaying(FireMontage))
		{
			//UE_LOG(LogTemp, Log, TEXT("ASpaceCharacter::PlayFireMontage: --- Playing FireMontage! ---"));
			AnimInstance->Montage_Play(FireMontage);
		}
	}
}

void ASpaceCharacter::StartCharge()
{
	ChangeState(ECharacterState::Charging);
}
void ASpaceCharacter::TryExecutionInput()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("ASpaceCharacter::TryExecutionInput: Execution Input Triggered"));

	if (ExecutionComp)
		if (ExecutionComp->StartExecution())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("ASpaceCharacter::TryExecutionInput: Execution Started"));
			if (ExecuteMontage)
				PlayAnimMontage(ExecuteMontage);
		}
}

void ASpaceCharacter::OnExecutionStart(AActor* Target)
{
	// 움직임 0으로
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->Velocity = FVector::ZeroVector;
	}
	
	// 2) 입력 잠금
	DisableInput(Cast<APlayerController>(Controller));
	
	FVector ExecPos = GetExecutionPosition(Target, 500.f, 400.f);
	
	SetActorLocation(ExecPos);

	// ================================
	//  적을 바라보도록 회전
	// ================================
	FRotator LookAt = (Target->GetActorLocation() - GetActorLocation()).Rotation();
	SetActorRotation(LookAt);
	Controller->SetControlRotation(LookAt);
	//  카메라 확대
	FollowCamera->SetFieldOfView(70.f);
	// ================================
	//  VFX 발사 (예: Teleport VFX)
	// ================================
	if (ExecutionTeleportVFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExecutionTeleportVFX,
			GetActorLocation(),
			GetActorRotation()
		);
	}
	// 3) 스프링암 살짝 당기기 (옵션)
	CameraBoom->TargetArmLength = 150.f;

	// 4) 스프링암 Offset 살짝 조정
	CameraBoom->SocketOffset = FVector(0, 40, 20);

	bIsCameraTransitioning = true;
}


void ASpaceCharacter::OnExecutionEnd(AActor* Target)
{
	FollowCamera->SetFieldOfView(90.f);
	bIsCameraTransitioning = true;

	EnableInput(Cast<APlayerController>(Controller));
}


float ASpaceCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	float RealHealthDamageTaken = 0.f;
	/*
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ASpaceCharacter::TakeDamage: DamageAmount = %f, ActualDamage = %f"), DamageAmount, ActualDamage));
	*/
	if (HealthComp)
	{
		RealHealthDamageTaken = HealthComp->ApplyHealthDamage(ActualDamage);
	}

	if (!bIsDead && RealHealthDamageTaken > 0.f && HitMontage)
	{
		PlayAnimMontage(HitMontage);
	}

	return ActualDamage;
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
}
void ASpaceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ReloadAction)
			EnhancedInput->BindAction(ReloadAction, ETriggerEvent::Started, this, &ASpaceCharacter::HandleReload);
		if (MoveAction)
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASpaceCharacter::Move);

		if (SprintAction)
		{
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASpaceCharacter::HandleSprintOrBoostInput);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASpaceCharacter::StopSprint);
		}

		if (LookAction)
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceCharacter::Look);

		if(ShieldAction)
			EnhancedInput->BindAction(ShieldAction, ETriggerEvent::Started, this, &ASpaceCharacter::OnShieldKeyPressed);

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
			EnhancedInput->BindAction(BoostAction, ETriggerEvent::Started, this, &ASpaceCharacter::HandleSprintOrBoostInput);
		}
		if(ExecuteAction)
			EnhancedInput->BindAction(ExecuteAction, ETriggerEvent::Started, this, &ASpaceCharacter::TryExecutionInput);
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

FVector ASpaceCharacter::GetExecutionPosition(AActor* Target, float ForwardOffset, float UpOffset)
{
	if (!Target) return GetActorLocation();

	// 1) 적의 정면 방향
	FVector Forward = Target->GetActorForwardVector().GetSafeNormal();

	// 2) 적의 바닥 위치 기준
	const float HalfHeight = Target->GetSimpleCollisionHalfHeight();
	FVector BaseLocation = Target->GetActorLocation() - FVector(0, 0, HalfHeight);

	// 3) 최종 위치: 적 정면 ForwardOffset + 위로 UpOffset
	FVector ExecPos =
		BaseLocation +
		Forward * ForwardOffset +   // 적 정면 앞으로 이동
		FVector(0, 0, UpOffset);    // 위로 Offset

	return ExecPos;
}
