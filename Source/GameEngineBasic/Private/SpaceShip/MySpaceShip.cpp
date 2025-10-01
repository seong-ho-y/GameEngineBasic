
#include "SpaceShip/MySpaceShip.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"

// 기본 생성자
AMySpaceShip::AMySpaceShip()
{
	PrimaryActorTick.bCanEverTick = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh; // 루트 컴포넌트로 설정

	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->BodyInstance.bEnableGravity = false;
	ShipMesh->SetEnableGravity(false);
	
	// 선형 감쇠 (직선 움직임 저항)
	// 값이 높을수록 더 빨리 멈춥니다. 우주선처럼 미끄러지는 느낌을 주려면 낮은 값을 사용하세요.
	ShipMesh->SetLinearDamping(0.5f);
	
	// 각속도 감쇠 (회전 움직임 저항)
	// 값이 높을수록 회전이 더 빨리 멈춥니다.
	ShipMesh->SetAngularDamping(4.0f);


	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;

	// --- 아래 카메라 지연 옵션 활성화 ---
	SpringArm->bEnableCameraLag = true;         // 위치 지연 활성화
	SpringArm->CameraLagSpeed = 5.0f;           // 위치 지연 속도

	SpringArm->bEnableCameraRotationLag = true; // 회전 지연 활성화
	SpringArm->CameraRotationLagSpeed = 5.0f;   // 회전 지연 속도

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	
}

// 게임 시작 시 호출되는 함수
void AMySpaceShip::BeginPlay()
{
	Super::BeginPlay();

}

// 매 프레임마다 호출되는 함수
void AMySpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. 현재 우주선의 회전 값을 가져옵니다. (물리 엔진에 의해 계산된 값)
	const FRotator CurrentRotation = GetActorRotation();

	// 2. 목표 기울기 각도를 계산합니다. (마우스 X 입력에 비례)
	const float TargetRollAngle = CurrentLookInput.X * MaxRollAngle;

	// 3. 현재 기울기 각도에서 목표 각도로 부드럽게 보간(Interpolation)합니다.
	const float NewRollAngle = FMath::FInterpTo(CurrentRotation.Roll, TargetRollAngle, DeltaTime, RollSpeed);

	// 4. 물리 엔진이 계산한 Pitch, Yaw 값은 유지하고, Roll 값만 새로 계산한 값으로 변경하여 새로운 회전 값을 만듭니다.
	const FRotator NewRotation = FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRollAngle);

	// 5. 최종 회전 값을 우주선에 적용합니다.
	SetActorRotation(NewRotation);
}

// 플레이어 입력 처리
void AMySpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AMySpaceShip::MoveForward);
		EnhancedInput->BindAction(IA_MoveUpDown, ETriggerEvent::Triggered, this, &AMySpaceShip::MoveUpDown);
		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Triggered, this, &AMySpaceShip::Boost);
		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMySpaceShip::Look);
	}

	else {
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found!"));
	}
}

void AMySpaceShip::MoveForward(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();
	float CurrentThrust = ThrustForce;

	// 부스트 상태라면 추력을 증가시킵니다.
	if (bIsBoosting)
	{
		CurrentThrust *= BoostMultiplier;
	}

	// 우주선의 앞 방향으로 힘을 가합니다.
	// AddForce의 마지막 인자(bAccelChange)를 true로 하면 질량에 상관없이 일정한 가속도를 냅니다.
	ShipMesh->AddForce(GetActorForwardVector() * InputValue * CurrentThrust, NAME_None, true);

}

void AMySpaceShip::MoveUpDown(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();

	// 우주선의 위쪽 방향으로 힘을 가합니다.
	ShipMesh->AddForce(GetActorUpVector() * InputValue * UpDownForce, NAME_None, true);

}

void AMySpaceShip::Look(const FInputActionValue& Value)
{
	// 입력 값을 FVector2D로 읽어옵니다.
	CurrentLookInput = Value.Get<FVector2D>();

	// Pitch (상하 회전) 토크: 마우스 Y 입력 * 우주선 오른쪽 벡터
	// 마우스 Y는 보통 위로 올리면 -값이므로, -를 곱해 방향을 맞춰줍니다.
	FVector PitchTorque = GetActorRightVector() * -CurrentLookInput.Y * TurnTorque;

	// Yaw (좌우 회전) 토크: 마우스 X 입력 * 우주선 위쪽 벡터
	FVector YawTorque = GetActorUpVector() * CurrentLookInput.X * TurnTorque;

	// 두 토크를 합쳐서 적용합니다.
	ShipMesh->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, true);
}

void AMySpaceShip::Boost()
{
	bIsBoosting = !bIsBoosting;
}