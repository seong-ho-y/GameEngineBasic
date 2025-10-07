
#include "SpaceShip/MySpaceShip.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

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
	SpringArm->CameraLagSpeed = 8.0f;

	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 8.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	
}

// 게임 시작 시 호출되는 함수
void AMySpaceShip::BeginPlay()
{
	Super::BeginPlay();
	InitialYaw = GetActorRotation().Yaw; // 시작 헤딩 기억
}

// 매 프레임마다 호출되는 함수
void AMySpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyUpright(DeltaTime);
	ApplyBankControl(DeltaTime);
	ClampSpeeds();
}

// 플레이어 입력 처리
void AMySpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AMySpaceShip::MoveForward);

		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Triggered, this, &AMySpaceShip::Boost);

		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMySpaceShip::Look);
		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Completed, this, &AMySpaceShip::LookEnded);
	}

	else {
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found!"));
	}
}

void AMySpaceShip::MoveForward(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (FMath::IsNearlyZero(Axis))
	{
		return;
	}

	float CurrentThrust = ThrustForce * Axis;
	if (bIsBoosting)
	{
		CurrentThrust *= BoostMultiplier;
	}

	// 질량 무시하고 가속으로(가벼운 조작감)
	ShipMesh->AddForce(GetActorForwardVector() * CurrentThrust, NAME_None, /*bAccelChange=*/true);
}

void AMySpaceShip::Look(const FInputActionValue& Value)
{
	// 2D 축(-Y = 위)
	CurrentLookInput = Value.Get<FVector2D>();

	// 피치/요는 즉시 토크로 반영
	const FVector PitchTorque = GetActorRightVector() * (-CurrentLookInput.Y * TurnTorque);
	const FVector YawTorque = GetActorUpVector() * (CurrentLookInput.X * TurnTorque);

	ShipMesh->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, /*bAccelChange=*/true);
}

void AMySpaceShip::LookEnded(const FInputActionValue& /*Value*/)
{
	CurrentLookInput = FVector2D::ZeroVector; // 입력 끝났을 때 자연감 복귀
}

void AMySpaceShip::Boost()
{
	bIsBoosting = !bIsBoosting;
}

void AMySpaceShip::ApplyBankControl(float DeltaTime)
{
	// 마우스 X에 비례한 목표 롤 각도(도)
	const float TargetRoll = FMath::Clamp(CurrentLookInput.X, -1.0f, 1.0f) * MaxRollAngle;

	// 현재 롤 각(도)
	const float CurrentRoll = GetActorRotation().Roll;

	// 오차
	const float Error = FMath::FindDeltaAngleDegrees(CurrentRoll, TargetRoll);

	// 현재 각속도(도/초)에서 "롤 축 성분"만 추출 (롤 축=포워드 벡터)
	const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const FVector Fwd = GetActorForwardVector();
	const float RollRate = FVector::DotProduct(AngVelDeg, Fwd); // 도/초

	// 간단한 PD 제어: Torque = Kp * error - Kd * rate
	const float Control = (BankKp * Error) - (BankKd * RollRate);

	// 롤 축 토크 적용(도 단위)
	const FVector RollTorque = Fwd * Control;
	ShipMesh->AddTorqueInDegrees(RollTorque, NAME_None, /*bAccelChange=*/true);
}

void AMySpaceShip::ClampSpeeds() const
{
	// 선속 제한
	const FVector V = ShipMesh->GetPhysicsLinearVelocity();
	const float Speed = V.Length();
	if (Speed > MaxLinearSpeed)
	{
		const FVector Clamped = V.GetSafeNormal() * MaxLinearSpeed;
		ShipMesh->SetPhysicsLinearVelocity(Clamped);
	}

	// 각속 제한(도/초)
	const FVector AV = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const float AVMag = AV.Length();
	if (AVMag > MaxAngularSpeed)
	{
		const FVector ClampedAV = AV.GetSafeNormal() * MaxAngularSpeed;
		ShipMesh->SetPhysicsAngularVelocityInDegrees(ClampedAV);
	}
}

void AMySpaceShip::ApplyUpright(float DeltaTime)
{
    // 원하는 Up = 월드 Up(0,0,1)
    const FVector DesiredUp = FVector::UpVector;
    const FVector CurrUp    = GetActorUpVector();

    // 최단 회전축(방향)과 각도 구하기
    // 축 = CurrUp x DesiredUp (오른손법칙), 크기 = sin(theta)
    FVector  Axis = FVector::CrossProduct(CurrUp, DesiredUp);
    const float SinTheta = Axis.Length();
    const float CosTheta = FVector::DotProduct(CurrUp, DesiredUp);

    // 이미 거의 수직이면 스킵
    if (SinTheta < 1e-3f && CosTheta > 0.999f)
        return;

    Axis = Axis.GetSafeNormal();

    // 각도(도) = atan2(sin, cos) -> 라디안->도 변환
    const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(SinTheta, CosTheta));

    // 현재 각속도(도/초)에서 "교정축" 성분만 추출해 D항에 사용
    const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
    const float   RateAlongAxis = FVector::DotProduct(AngVelDeg, Axis);

    // PD 제어: 축 방향 토크 (도 단위 토크 사용)
    const float Control = (UprightKp * AngleDeg) - (UprightKd * RateAlongAxis);
    const FVector UprightTorque = Axis * Control;
    ShipMesh->AddTorqueInDegrees(UprightTorque, NAME_None, /*bAccelChange=*/true);

    // --- (옵션) 시작 헤딩으로 천천히 복귀 ---
    if (bRestoreYawToInitial)
    {
        // 수평면에서의 진행방향으로만 비교(롤/피치 영향 제거)
        const FRotator Rot = GetActorRotation();

        const float CurrYaw = Rot.Yaw;
        const float YawErr  = FMath::FindDeltaAngleDegrees(CurrYaw, InitialYaw);

        // 현재 각속도에서 'Up 축' 성분만 추출(= 요 레이트)
        const float YawRate = FVector::DotProduct(AngVelDeg, FVector::UpVector);

        const float YawControl = (YawKp * YawErr) - (YawKd * YawRate);
        const FVector YawTorque = FVector::UpVector * YawControl;

        ShipMesh->AddTorqueInDegrees(YawTorque, NAME_None, /*bAccelChange=*/true);
    }
}