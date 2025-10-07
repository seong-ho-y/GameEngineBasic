
#include "SpaceShip/MyTestPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyShipMovement/MyShipMovement.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

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

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;

	// 카메라 지연(과하지 않게)
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 60.f; // 1) 속도 크게 올려서 빨리 붙도록
	SpringArm->CameraLagMaxDistance = 5.f; // 2) 최대 지연거리 크게 줄여서 “길게 뒤쳐짐” 방지

	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->CameraRotationLagSpeed = 20.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	ShipMovement = CreateDefaultSubobject<UMyShipMovement>(TEXT("ShipMovement"));
}

// 게임 시작 시 호출되는 함수
void AMyTestPawn::BeginPlay()
{
	Super::BeginPlay();

	if (ShipMovement && ShipMesh)
	{
		const float InitialYaw = GetActorRotation().Yaw;
		ShipMovement->Initialize(ShipMesh, InitialYaw);
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

		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Started, this, &AMyTestPawn::Boost);

		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMyTestPawn::Look);
		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Completed, this, &AMyTestPawn::LookEnded);
	}

	else {
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found!"));
	}
}

void AMyTestPawn::MoveForward(const FInputActionValue& Value)
{
	if (ShipMovement) { ShipMovement->MoveForward(Value); }
}

void AMyTestPawn::Look(const FInputActionValue& Value)
{
	if (ShipMovement) { ShipMovement->Look(Value); }
}

void AMyTestPawn::LookEnded(const FInputActionValue& Value)
{
	if (ShipMovement) { ShipMovement->LookEnded(Value); }
}

void AMyTestPawn::Boost()
{
	if (ShipMovement) { ShipMovement->Boost(); }
}