// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/SpaceCharacter.h"
#include "MyPlayerController.h"
#include "GameEngineBasic/System/SaveSystemManager.h"

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

#include "InventoryComponent.h"
#include "MyPlayerState.h"
#include "PlayerStatsComponent.h"
#include "WeaponComponent.h"

class AMyPlayerState;

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

	StatsComp = CreateDefaultSubobject<UPlayerStatsComponent>(TEXT("PlayerStatsComp"));
	Shooter = CreateDefaultSubobject<UShooterComp>(TEXT("ShooterComp"));
	Fuel = CreateDefaultSubobject<UFuelComponent>(TEXT("FuelComp"));
	WingComp = CreateDefaultSubobject<UWingComponent>(TEXT("WingComp"));
	ShieldComp = CreateDefaultSubobject<UShieldComp>(TEXT("ShieldComp"));
	HealthComp = CreateDefaultSubobject<UHealthComp>(TEXT("HealthComp"));
	TargetingComp = CreateDefaultSubobject<UTargetingSystemComponent>(TEXT("TargetingComp"));

	ExecutionComp = CreateDefaultSubobject<UExecutionComp>(TEXT("ExecutionComp"));

	DashVfx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DashVfx"));
	DashVfx->SetupAttachment(GetMesh());
	DashVfx->bAutoActivate = false;

	HealVfx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealVfx"));
	HealVfx->SetupAttachment(GetMesh());
	HealVfx->bAutoActivate = false;

	WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComp"));
}

void ASpaceCharacter::InitFromPlayerState()
{
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[InitFromPlayerState] 호출됨"));

    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	/*
    if (!PS)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[InitFromPlayerState] PlayerState 없음"));
    }
    else
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[InitFromPlayerState] PlayerState 존재"));
    }
	*/
    if (!PS || !PS->Inventory)
    {
        //if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[InitFromPlayerState] Inventory 없음 -> 다음 틱 재시도"));

        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimerForNextTick(
                FTimerDelegate::CreateUObject(this, &ASpaceCharacter::InitFromPlayerState)
            );
        }
        return;
    }
    if (PS->Inventory)
	{
    	PS->Inventory->CachedStats = StatsComp;
    }
	StatsComp->ApplyParts();

    // Weapon
    if (!WeaponComp || !Shooter)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[InitFromPlayerState] WeaponComp 또는 Shooter == NULL !!"));
        return;
    }

    //if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("[InitFromPlayerState] WeaponComp & Shooter OK"));

    FName WeaponRow = PS->Inventory->GetCurrentWeapon();
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan,
        FString::Printf(TEXT("[InitFromPlayerState] GetCurrentWeapon = %s"), *WeaponRow.ToString()));

    if (WeaponRow == NAME_None)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("[InitFromPlayerState] WeaponRow == None !!!!!"));
        return;
    }

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("[InitFromPlayerState] InitializeWeapon 호출 준비"));

    WeaponComp->WeaponRowName = WeaponRow;

    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
        FString::Printf(TEXT("[InitFromPlayerState] InitializeWeapon 실행!!! Row=%s"), *WeaponRow.ToString()));

    WeaponComp->InitializeWeapon(this, Shooter);
}

void ASpaceCharacter::BeginPlay()
{
	//USaveSystemManager::LoadPawnState(this);
	Super::BeginPlay();
	
	InitFromPlayerState();
	
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

	UAnimInstance* Anim = GetMesh()->GetAnimInstance();
	Anim->Montage_Play(LevelStartMontage);
	DisableInputForDuration(5.0f);
}

void ASpaceCharacter::DisableInputForDuration(float Duration)
{
	// 2초간 입력을 비활성화
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(true); // 이동 입력 무시
		PlayerController->SetIgnoreLookInput(true); // 회전 입력 무시
	}

	// 일정 시간이 지나면 입력을 다시 활성화
	GetWorld()->GetTimerManager().SetTimer(InputDisableTimerHandle, this, &ASpaceCharacter::EnableInputAfterDelay, Duration, false);
}

void ASpaceCharacter::EnableInputAfterDelay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		PlayerController->SetIgnoreMoveInput(false); // 이동 입력 활성화
		PlayerController->SetIgnoreLookInput(false); // 회전 입력 활성화
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
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Started, this, &ASpaceCharacter::OnSprintPressed);
			EnhancedInput->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASpaceCharacter::OnSprintReleased);
		}

		if(DashAction)
			EnhancedInput->BindAction(DashAction, ETriggerEvent::Started, this, &ASpaceCharacter::StartDash);

		if (LookAction)
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASpaceCharacter::Look);

		if (ShieldAction)
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
			EnhancedInput->BindAction(BoostAction, ETriggerEvent::Started, this, &ASpaceCharacter::StartBoost);
		}
		if (ExecuteAction)
			EnhancedInput->BindAction(ExecuteAction, ETriggerEvent::Started, this, &ASpaceCharacter::TryExecutionInput);
		if (InteractAction)
			EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ASpaceCharacter::TryInteract);
		if (SwapWeaponAction)
		{
			EnhancedInput->BindAction(SwapWeaponAction, ETriggerEvent::Started, this, &ASpaceCharacter::SwapWeapon);
		}
	}
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

		const float UpLaunchPower = 5000.f;

		LaunchCharacter(FVector::UpVector * UpLaunchPower, false, false);

		if (FlyUpMontage) {
			Anim->Montage_Play(FlyUpMontage);
			WingComp->PlayFly();
			FVector SpawnLoc = GetOwner()->GetActorLocation();
			UGameplayStatics::PlaySoundAtLocation(this, JumpSound, SpawnLoc);
			return;
		}
	}

	
	Jump();
}

void ASpaceCharacter::StopJump()
{
	if (CurrentState == ECharacterState::Flying)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Stop(0.1f, FlyUpMontage);
			WingComp->StopAll();
		}
	}

	StopJumping();
	
}

void ASpaceCharacter::OnSprintPressed() // Sprint 판단
{
	bSprintHeld = true;

	GetWorldTimerManager().SetTimer(
		SprintHoldTimer,
		this,
		&ASpaceCharacter::StartSprint,
		0.2f,
		false
	);
	
}

void ASpaceCharacter::OnSprintReleased()
{
	bSprintHeld = false;
	/*
	if (GetWorldTimerManager().IsTimerActive(SprintHoldTimer))
	{
		GetWorldTimerManager().ClearTimer(SprintHoldTimer);
		StartDash();
		return;
	}
	*/

	StopSprint();
}

void ASpaceCharacter::StartSprint()
{
	AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->CanUseAbility(EAbilityType::Sprint)) return;

	bIsSprinting = true;
	TargetSpeed = RunSpeed;
	WingComp->PlaySprint();
}

void ASpaceCharacter::StopSprint()
{
	bIsSprinting = false;
	TargetSpeed = WalkSpeed;
	WingComp->StopAll();
}

void ASpaceCharacter::StartDash()
{
	if (!Fuel->CanDash())
		return;
	AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->CanUseAbility(EAbilityType::Dash))
		return;

	if (GetCharacterMovement()->IsFalling() || bIsFlyingMode)
		return;

	GetFuelComponent()->ConsumeDash(10);
	StartDashEffect();

	bIsDashing = true;
	UCharacterMovementComponent* Move = GetCharacterMovement();

	FVector DashDir = GetDashDirection();
	Move->Velocity = DashDir * 5000;

	// 대쉬 중 방향고정
	Move->bOrientRotationToMovement = false;
	SetActorRotation(DashDir.Rotation());
	Move->GroundFriction = 1.f;

	GetWorldTimerManager().SetTimer(
		DashTimerHandle,
		this,
		&ASpaceCharacter::StopDash,
		1.0f,
		false
	);

	FVector SpawnLoc = GetOwner()->GetActorLocation();
	UGameplayStatics::PlaySoundAtLocation(this,DashSound, SpawnLoc);
}

void ASpaceCharacter::StopDash()
{
	bIsDashing = false;
}

void ASpaceCharacter::StartDashEffect()
{
	if (DashVfx)
	{
		DashVfx->Activate(true);
		DashVfx->SetActive(true, true);
	}

	GetWorldTimerManager().SetTimer(
		DashEffectTimerHandle,
		this,
		&ASpaceCharacter::StopDashEffect,
		0.2,
		false
	);
}

void ASpaceCharacter::StopDashEffect()
{
	if (DashVfx)
	{
		DashVfx->Deactivate();
	}
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	Move->bOrientRotationToMovement = true;
	Move->GroundFriction = 8.f;
}

FVector ASpaceCharacter::GetDashDirection() const
{
	FVector InputDir = GetLastMovementInputVector();
	if (InputDir.IsNearlyZero())
		InputDir = GetActorForwardVector();
	InputDir.Normalize();

	return InputDir;
}

void ASpaceCharacter::StartBoost()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Boost Input Received"));

	AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->CanUseAbility(EAbilityType::Boost))
		return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("Boost Ability Allowed"));

	auto Move = GetCharacterMovement();
	if (!Move) return;

	const bool bAir = Move->IsFalling();
	const bool bFly = (CurrentState == ECharacterState::Flying) || bIsFlyingMode;

	if (bAir || bFly)
	{
		ChangeState(ECharacterState::Boosting);
		return;
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
	AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->CanUseAbility(EAbilityType::Shield))
		return;

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

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
		{
			MyPC->ShowDeathWidget();   // 새 함수
		}
	}
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShieldComp->DeactivateShield();
	}
	
	if (DeathMontage)
	{
		// 몽타주를 재생하고 총 길이를 반환받습니다.
		float MontageLength = PlayAnimMontage(DeathMontage);

		// 몽타주 데이터에서 BlendOut 시간을 가져오면 더 정확하지만, 
		// 보통 끝부분에서 Idle로 섞이는 것을 방지하기 위해 
		// 총 길이보다 약간 일찍(예: 0.2초 전) 멈추거나, 
		// 몽타주 에셋 설정에서 'Enable Auto Blend Out'을 꺼야 합니다.
		// 여기서는 코드로 마지막 프레임 직전에 멈추도록 설정합니다.

		float FreezeDelay = (MontageLength > 0.25f) ? (MontageLength - 0.05f) : MontageLength;

		// 타이머 설정: 애니메이션이 끝나기 직전에 FreezeDeathPose 함수 호출
		GetWorldTimerManager().SetTimer(
			DeathTimerHandle,
			this,
			&ASpaceCharacter::ExplodeAndDestroy,
			FreezeDelay,
			false
		);

		// 사망 사운드 재생
		FVector SpawnLoc = GetOwner()->GetActorLocation();
		UGameplayStatics::PlaySoundAtLocation(this, DeathSound, SpawnLoc);
	}
}

void ASpaceCharacter::ExplodeAndDestroy()
{
	GetMesh()->bPauseAnims = true;

	if (DeathExplosionEffect && GetMesh())
	{
		UGameplayStatics::SpawnEmitterAttached(
			DeathExplosionEffect,
			GetMesh(),
			FName("Die")
		);
	}

	Destroy();
}

void ASpaceCharacter::ToggleFlyingMode()
{
	AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->CanUseAbility(EAbilityType::Flying))
		return;

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

void ASpaceCharacter::StartAim()
{
	if (CurrentState == ECharacterState::Flying || bIsFlyingMode)
		return;

	bIsAiming = true;
	bIsCameraTransitioning = true;

	if (CurrentState == ECharacterState::Locomotion)
		ChangeState(ECharacterState::Aiming);
}

void ASpaceCharacter::StopAim()
{
	bIsAiming = false;
	bIsCameraTransitioning = true; // 카메라 줌 아웃을 위해 트랜지션 시작

	ChangeState(ECharacterState::Locomotion);
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
			PlaySingleFireMontage();
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
		PlayChargeFireMontage();
		return;
	}
	*/
}

void ASpaceCharacter::PlayChargeFireMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (ChargeFireMontage && !AnimInstance->Montage_IsPlaying(ChargeFireMontage))
		{
			//UE_LOG(LogTemp, Log, TEXT("ASpaceCharacter::ChargeFireMontage: --- Playing FireMontage! ---"));
			AnimInstance->Montage_Play(ChargeFireMontage);
		}
	}
}

void ASpaceCharacter::PlaySingleFireMontage()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (SingleFireMontage && !AnimInstance->Montage_IsPlaying(SingleFireMontage))
		{
			//UE_LOG(LogTemp, Log, TEXT("ASpaceCharacter::SingleFireMontage: --- Playing SingleFireMontage! ---"));
			AnimInstance->Montage_Play(SingleFireMontage);
		}
	}
}

void ASpaceCharacter::StartCharge()
{
	ChangeState(ECharacterState::Charging);
}

void ASpaceCharacter::TryExecutionInput()
{
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

	//체력 회복 & 탄약 회복
	HealthComp->CurrentHealth = FMath::Max(HealthComp->CurrentHealth+=50, HealthComp->MaxHealth);
	Shooter->CurrentAmmo = Shooter->FullAmmo;
	
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

void ASpaceCharacter::TryInteract()
{
	if (!CurrentInteractTarget)
		return;

	CurrentInteractTarget->Interact(this);
	CurrentInteractTarget = nullptr;
}


void ASpaceCharacter::UnlockAbility(EAbilityType Ability)
{
	if (AMyPlayerState* PS = GetPlayerState<AMyPlayerState>())
	{
		PS->UnlockAbility(Ability);
	}

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
void ASpaceCharacter::SwapWeapon()
{
	if (!WeaponComp || !Shooter) return;

	AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	if (!PS) return;
	// 0) 현재 무기 Mesh 제거 (시각적 잔상 방지)
	WeaponComp->ClearWeaponMesh();

	// 1) 현재 무기 런타임 상태 저장 (탄약 등)
	WeaponComp->SaveRuntimeState();

	// 2) 다음 RowName 결정 (순환 구조)
	const FName OldRow = WeaponComp->WeaponRowName;
	FName NewRow;

	if (OldRow == FName("HandgunBasic") && PS->IsWeaponUnlocked("RifleBasic"))
	{
		NewRow = FName("RifleBasic");
	}
	else if (OldRow == FName("RifleBasic") && PS->IsWeaponUnlocked("BlastBasic"))
	{
		NewRow = FName("BlastBasic");
	}
	else if (OldRow == FName("BlastBasic") && PS->IsWeaponUnlocked("ShotgunBasic"))
	{
		NewRow = FName("ShotgunBasic");
	}
	else NewRow = FName("HandgunBasic");

	// 3) RowName 변경만 해주고
	WeaponComp->WeaponRowName = NewRow;

	// 4) 다시 Initialize → DT 로딩 + RuntimeState 로딩 + Behavior 선택 + Mesh 스폰 + HUD 갱신
	WeaponComp->InitializeWeapon(this, Shooter);

	GEngine->AddOnScreenDebugMessage(
		-1, 2.f, FColor::Green,
		FString::Printf(TEXT("Swapped to %s"), *WeaponComp->WeaponRowName.ToString())
	);
}
