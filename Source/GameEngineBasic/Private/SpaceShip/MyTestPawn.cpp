
#include "SpaceShip/MyTestPawn.h"
#include "MyShipMovement/MyShipMovement.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Component/ShieldComp.h"
#include "EnhancedInputComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameEngineBasic/Components/public/HealthComp.h"

#include "SpaceCharacter/Shield/ShieldActor.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameFramework/PlayerController.h"

#include "Projectile.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

// 기본 생성자
AMyTestPawn::AMyTestPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh;

	ShipMesh->SetSimulatePhysics(false);
	ShipMesh->SetEnableGravity(false);
	ShipMesh->SetCollisionProfileName(TEXT("ShipBody"));
	ShipMesh->SetLinearDamping(0.3f);
	ShipMesh->SetAngularDamping(1.2f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(ShipMesh);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 60.f;
	SpringArm->bEnableCameraRotationLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionComp->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionComp->InitSphereRadius(500.f);
	CollisionComp->SetupAttachment(RootComponent);

	ShieldComp = CreateDefaultSubobject<UShieldComp>(TEXT("ShieldComp"));
	ShipMovement = CreateDefaultSubobject<UMyShipMovement>(TEXT("ShipMovement"));
	Shooter = CreateDefaultSubobject<UShooterComp>(TEXT("ShooterComp"));
	HealthComp = CreateDefaultSubobject<UHealthComp>(TEXT("HealthComp"));
}

// 게임 시작 시 호출되는 함수
void AMyTestPawn::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComp)
	{
		HealthComp->OnHealthChanged_Ver2.AddDynamic(this, &AMyTestPawn::OnHealthChanged);
		HealthComp->OnDeath.AddDynamic(this, &AMyTestPawn::OnDeath);
	}
	if(ShieldComp)
	{
		ShieldComp->OnShieldActivated.AddDynamic(this, &AMyTestPawn::OnShieldActivated);
	}
	if (CollisionComp)
	{
		CollisionComp->OnComponentEndOverlap.AddDynamic(this, &AMyTestPawn::OnOverlapEnd);
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AMyTestPawn::OnOverlapBegin);

	}
}

// 매 프레임마다 호출되는 함수
void AMyTestPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 플레이어 입력 처리
void AMyTestPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AMyTestPawn::MoveForward);

		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Started, this, &AMyTestPawn::Boost_Pressed);
		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Completed, this, &AMyTestPawn::Boost_Released);

		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMyTestPawn::Look);

		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Started, this, &AMyTestPawn::FireStarted);
		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Triggered, this, &AMyTestPawn::FireTriggered);
		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AMyTestPawn::FireCompleted);
	
		EnhancedInput->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &AMyTestPawn::Roll);
		EnhancedInput->BindAction(IA_Roll, ETriggerEvent::Completed, this, &AMyTestPawn::Roll);
		
		EnhancedInput->BindAction(IA_Brake, ETriggerEvent::Started, this, &AMyTestPawn::Brake_Pressed);
		EnhancedInput->BindAction(IA_Brake, ETriggerEvent::Completed, this, &AMyTestPawn::Brake_Released);
	
		EnhancedInput->BindAction(IA_Shield, ETriggerEvent::Started, this, &AMyTestPawn::OnShieldKeyPressed);
	}

	else {
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found!"));
	}
}

void AMyTestPawn::Roll(const FInputActionValue& Value)
{
	if (ShipMovement) ShipMovement->Roll(Value);
}

void AMyTestPawn::MoveForward(const FInputActionValue& Value)
{
	if (ShipMovement) { ShipMovement->MoveForward(Value); }
}

void AMyTestPawn::Look(const FInputActionValue& Value)
{
	if (ShipMovement) { ShipMovement->Look(Value); }
}

void AMyTestPawn::Boost_Pressed()
{
	if (!ShipMovement) return;
	ShipMovement->StartBoost();
}

void AMyTestPawn::Boost_Released()
{
	if (!ShipMovement) return;
	ShipMovement->StopBoost();
}

void AMyTestPawn::Brake_Pressed()
{
	if (!ShipMovement) return;
	ShipMovement->StartBrake();
}

void AMyTestPawn::Brake_Released()
{
	if (!ShipMovement) return;
	ShipMovement->StopBrake();
}

void AMyTestPawn::FireTriggered(const FInputActionValue& /*Value*/)
{
	UE_LOG(LogTemp, Warning, TEXT("FireTriggered"));
	if (Shooter) Shooter->TryFire(); // 쿨다운이 끝났을 때만 실제 발사됨
}

void AMyTestPawn::FireStarted(const FInputActionValue& /*Value*/)
{
	UE_LOG(LogTemp, Warning, TEXT("FireStarted"));
}

void AMyTestPawn::FireCompleted(const FInputActionValue& /*Value*/)
{
	UE_LOG(LogTemp, Warning, TEXT("FireCompleted"));
}

void AMyTestPawn::OnShieldActivated()
{
	// 실드 활성화 처리
	UE_LOG(LogTemp, Warning, TEXT("Shield Activated!"));
}

void AMyTestPawn::OnShieldKeyPressed(const FInputActionInstance& Instance)
{
	if (ShieldComp)
	{
		ShieldComp->ActivateShield();
	}
}

void AMyTestPawn::OnShieldBroken(AActor* OwnerActor)
{
	// 실드 깨졌을 때 처리
	// 예: 경고 로그, UI 반영, 무적 타이머 등
	UE_LOG(LogTemp, Warning, TEXT("Shield Broken!"));
}

void AMyTestPawn::OnHealthChanged(float NewHealth, float MaxHealth)
{
	// HUD 업데이트 등
	UE_LOG(LogTemp, Log, TEXT("HP: %f  Max: %f"), NewHealth, MaxHealth);
}

void AMyTestPawn::OnDeath(AActor* OwnerActor)
{
	// 사망 처리: 입력 끄기, 폭발, 리스폰 트리거 등
	UE_LOG(LogTemp, Warning, TEXT("Pawn Died"));
	
	APlayerController* PC = Cast<APlayerController>(GetController());
	DisableInput(PC);

	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionFX,
			GetActorTransform(), // 위치/회전 그대로
			true                 // bAutoDestroy: 파티클 끝나면 자동 정리
		);
	}

	Destroy();
	
}


void AMyTestPawn::Interact(ASpaceCharacter* Character)
{
	if (!Character) return;

	APlayerController* PC = Cast<APlayerController>(Character->GetController());
	if (!PC) return;

	// 1) 캐릭터 입력 중지
	Character->DisableInput(PC);

	// 2) 캐릭터 숨기기 및 충돌 끄기
	Character->SetActorHiddenInGame(true);
	Character->SetActorEnableCollision(false);

	// 3) PlayerController가 우주선 Possess
	PC->Possess(this);
	ShipMesh->SetSimulatePhysics(true);

	// 4) 우주선 조종 시작
	EnableInput(PC);

	UE_LOG(LogTemp, Log, TEXT("Player Boarding Ship!"));
}

void AMyTestPawn::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		Character->CurrentInteractTarget = this;
		// 필요하면 InteractWidget 표시
	}
}

void AMyTestPawn::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		if (Character->CurrentInteractTarget == this)
			Character->CurrentInteractTarget = nullptr;
	}
}