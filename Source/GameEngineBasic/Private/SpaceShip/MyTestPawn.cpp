
#include "SpaceShip/MyTestPawn.h"
#include "MyShipMovement/MyShipMovement.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"

#include "EnhancedInputComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"

#include "Projectile.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

// 기본 생성자
AMyTestPawn::AMyTestPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh; // 루트 컴포넌트로 설정

	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->BodyInstance.bEnableGravity = false;
	ShipMesh->SetEnableGravity(false);

	// 선형 감쇠 (직선 움직임 저항)
	// 값이 높을수록 더 빨리 멈춥니다. 우주선처럼 미끄러지는 느낌을 주려면 낮은 값을 사용하세요.
	ShipMesh->SetLinearDamping(0.3f);

	// 각속도 감쇠 (회전 움직임 저항)
	// 값이 높을수록 회전이 더 빨리 멈춥니다.
	ShipMesh->SetAngularDamping(1.2f);

	ShipMesh->SetCollisionProfileName(TEXT("ShipBody")); // 선체

	ShieldComp = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldComp"));
	ShieldComp->SetupAttachment(ShipMesh);
	ShieldComp->InitSphereRadius(300.f); // 선체보다 살짝 크게
	ShieldComp->SetCollisionProfileName(TEXT("Shield")); // 에디터에서 만든 Preset
	ShieldComp->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;

	// 카메라 지연(과하지 않게)
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 60.f; // 1) 속도 크게 올려서 빨리 붙도록
	SpringArm->bEnableCameraRotationLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

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
		HealthComp->OnShieldBroken.AddDynamic(this, &AMyTestPawn::OnShieldBroken);
		HealthComp->OnHealthChanged.AddDynamic(this, &AMyTestPawn::OnHealthChanged);
		HealthComp->OnDeath.AddDynamic(this, &AMyTestPawn::OnDeath);
	}

	if (ShieldComp) 
	{
		ShieldComp->OnComponentBeginOverlap.AddDynamic(this, &AMyTestPawn::OnShieldOverlap);
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

void AMyTestPawn::OnShieldOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32, bool, const FHitResult&)
{
	if (OtherActor->ActorHasTag(TEXT("EnemyProjectile")))
	{
		if (HealthComp)
			HealthComp->TakeDamage();

		OtherActor->Destroy();
	}
}


void AMyTestPawn::OnShipHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == this) return;
	//OnDeath(this);
}

void AMyTestPawn::OnShieldBroken(AActor* OwnerActor)
{
	// 실드 깨졌을 때 처리
	// 예: 경고 로그, UI 반영, 무적 타이머 등
	UE_LOG(LogTemp, Warning, TEXT("Shield Broken!"));
}

void AMyTestPawn::OnHealthChanged(AActor* OwnerActor, float NewHealth, float NewShield)
{
	// HUD 업데이트 등
	UE_LOG(LogTemp, Log, TEXT("HP: %f  Shield: %f"), NewHealth, NewShield);
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