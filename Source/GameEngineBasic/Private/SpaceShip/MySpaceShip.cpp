
#include "SpaceShip/MySpaceShip.h"
#include "MyShipMovement/MyShipMovement.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Component/ShieldComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "EnhancedInputComponent.h"

#include "SpaceCharacter/Shield/ShieldActor.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"

#include "Projectile.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

// 기본 생성자
AMySpaceShip::AMySpaceShip()
{
	PrimaryActorTick.bCanEverTick = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh; // 루트 컴포넌트로 설정

	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->SetEnableGravity(false);
	ShipMesh->SetCollisionProfileName(TEXT("ShipBody"));
	ShipMesh->SetLinearDamping(0.3f);
	ShipMesh->SetAngularDamping(1.2f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 60.f;
	SpringArm->bEnableCameraRotationLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ShieldActorComp = CreateDefaultSubobject<UChildActorComponent>(TEXT("ShieldActorComp"));
	ShieldComp = CreateDefaultSubobject<UShieldComp>(TEXT("ShieldComp"));
	ShieldActorComp->SetupAttachment(ShipMesh);
	ShieldActorComp->SetChildActorClass(AShieldActor::StaticClass());

	ShipMovement = CreateDefaultSubobject<UMyShipMovement>(TEXT("ShipMovement"));
	Shooter = CreateDefaultSubobject<UShooterComp>(TEXT("ShooterComp"));
	HealthComp = CreateDefaultSubobject<UHealthComp>(TEXT("HealthComp"));
}

// 게임 시작 시 호출되는 함수
void AMySpaceShip::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComp)
	{
		HealthComp->OnHealthChanged_Ver2.AddDynamic(this, &AMySpaceShip::OnHealthChanged);
		HealthComp->OnDeath.AddDynamic(this, &AMySpaceShip::OnDeath);
	}

}

// 매 프레임마다 호출되는 함수
void AMySpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 플레이어 입력 처리
void AMySpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AMySpaceShip::MoveForward);

		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Started, this, &AMySpaceShip::Boost_Pressed);
		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Completed, this, &AMySpaceShip::Boost_Released);

		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMySpaceShip::Look);

		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Started, this, &AMySpaceShip::FireStarted);
		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Triggered, this, &AMySpaceShip::FireTriggered);
		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Completed, this, &AMySpaceShip::FireCompleted);

		EnhancedInput->BindAction(IA_Roll, ETriggerEvent::Triggered, this, &AMySpaceShip::Roll);
		EnhancedInput->BindAction(IA_Roll, ETriggerEvent::Completed, this, &AMySpaceShip::Roll);

		EnhancedInput->BindAction(IA_Brake, ETriggerEvent::Started, this, &AMySpaceShip::Brake_Pressed);
		EnhancedInput->BindAction(IA_Brake, ETriggerEvent::Completed, this, &AMySpaceShip::Brake_Released);
	}

	else {
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found!"));
	}
}

void AMySpaceShip::MoveForward(const FInputActionValue& Value)
{
	if (ShipMovement) { ShipMovement->MoveForward(Value); }
}

void AMySpaceShip::Look(const FInputActionValue& Value)
{
	if (ShipMovement) { ShipMovement->Look(Value); }
}

void AMySpaceShip::Roll(const FInputActionValue& Value)
{
	if (ShipMovement) ShipMovement->Roll(Value);
}

void AMySpaceShip::Boost_Pressed()
{
	if (!ShipMovement) return;
	ShipMovement->StartBoost();
}

void AMySpaceShip::Boost_Released()
{
	if (!ShipMovement) return;
	ShipMovement->StopBoost();
}

void AMySpaceShip::Brake_Pressed()
{
	if (!ShipMovement) return;
	ShipMovement->StartBrake();
}

void AMySpaceShip::Brake_Released()
{
	if (!ShipMovement) return;
	ShipMovement->StopBrake();
}

void AMySpaceShip::FireTriggered(const FInputActionValue& /*Value*/)
{
	UE_LOG(LogTemp, Warning, TEXT("FireTriggered"));
	if (Shooter) Shooter->TryFire(); // 쿨다운이 끝났을 때만 실제 발사됨
}

void AMySpaceShip::FireStarted(const FInputActionValue& /*Value*/)
{
	UE_LOG(LogTemp, Warning, TEXT("FireStarted"));
}

void AMySpaceShip::FireCompleted(const FInputActionValue& /*Value*/)
{
	UE_LOG(LogTemp, Warning, TEXT("FireCompleted"));
}

void AMySpaceShip::OnShieldBroken(AActor* OwnerActor)
{
	// 실드 깨졌을 때 처리
	// 예: 경고 로그, UI 반영, 무적 타이머 등
	UE_LOG(LogTemp, Warning, TEXT("Shield Broken!"));
}

void AMySpaceShip::OnHealthChanged(float NewHealth, float MaxHealth)
{
	// HUD 업데이트 등
	UE_LOG(LogTemp, Log, TEXT("HP: %f  Max: %f"), NewHealth, MaxHealth);
}

void AMySpaceShip::OnDeath(AActor* OwnerActor)
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