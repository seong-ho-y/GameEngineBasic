
#include "SpaceShip/MyTestPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

// �⺻ ������
AMyTestPawn::AMyTestPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh; // ��Ʈ ������Ʈ�� ����

	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->BodyInstance.bEnableGravity = false;
	ShipMesh->SetEnableGravity(false);

	// ���� ���� (���� ������ ����)
	// ���� �������� �� ���� ����ϴ�. ���ּ�ó�� �̲������� ������ �ַ��� ���� ���� ����ϼ���.
	ShipMesh->SetLinearDamping(0.3f);

	// ���ӵ� ���� (ȸ�� ������ ����)
	// ���� �������� ȸ���� �� ���� ����ϴ�.
	ShipMesh->SetAngularDamping(1.2f);


	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;

	// ī�޶� ����(������ �ʰ�)
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 8.0f;

	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 8.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);


}

// ���� ���� �� ȣ��Ǵ� �Լ�
void AMyTestPawn::BeginPlay()
{
	Super::BeginPlay();
	InitialYaw = GetActorRotation().Yaw; // ���� ��� ���
}

// �� �����Ӹ��� ȣ��Ǵ� �Լ�
void AMyTestPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ���� ���콺 �Է��� ũ��(0~1 ����)
	const float LookMag = CurrentLookInput.Length();

	// Deadzone�� �Ѿ�� '�Է� ����' �Ǵ�
	const bool bHasLook = LookMag > LookDeadzone;

	// �Է��� ������ Upright ���ϰ�(0), ������ ���ϰ�(1)
	const float TargetAlpha = bHasLook ? 0.0f : 1.0f;

	// ���� �ٸ��� ���� �ӵ� ����
	const float BlendSpeed = bHasLook ? UprightBlendOutSpeed : UprightBlendInSpeed;

	UprightAlpha = FMath::FInterpTo(UprightAlpha, TargetAlpha, DeltaTime, BlendSpeed);

	// Upright ����(�ִ� ���� ���� ����)
	ApplyUpright(DeltaTime, UprightAlpha * UprightMaxStrength);

	// �� PD ����� �ӵ� Ŭ������ �������
	ApplyBankControl(DeltaTime);
	ClampSpeeds();
}

// �÷��̾� �Է� ó��
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

	// ���� �����ϰ� ��������(������ ���۰�)
	ShipMesh->AddForce(GetActorForwardVector() * CurrentThrust, NAME_None, /*bAccelChange=*/true);
}

void AMyTestPawn::Look(const FInputActionValue& Value)
{
	// 2D ��(-Y = ��)
	CurrentLookInput = Value.Get<FVector2D>();

	// ��ġ/��� ��� ��ũ�� �ݿ�
	const FVector PitchTorque = GetActorRightVector() * (-CurrentLookInput.Y * TurnTorque);
	const FVector YawTorque = GetActorUpVector() * (CurrentLookInput.X * TurnTorque);

	ShipMesh->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, /*bAccelChange=*/true);
}

void AMyTestPawn::LookEnded(const FInputActionValue& /*Value*/)
{
	CurrentLookInput = FVector2D::ZeroVector; // �Է� ������ �� �ڿ��� ����
}

void AMyTestPawn::Boost()
{
	bIsBoosting = !bIsBoosting;
}

void AMyTestPawn::ApplyBankControl(float DeltaTime)
{
	// ���콺 X�� ����� ��ǥ �� ����(��)
	const float TargetRoll = FMath::Clamp(CurrentLookInput.X, -1.0f, 1.0f) * MaxRollAngle;

	// ���� �� ��(��)
	const float CurrentRoll = GetActorRotation().Roll;

	// ����
	const float Error = FMath::FindDeltaAngleDegrees(CurrentRoll, TargetRoll);

	// ���� ���ӵ�(��/��)���� "�� �� ����"�� ���� (�� ��=������ ����)
	const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const FVector Fwd = GetActorForwardVector();
	const float RollRate = FVector::DotProduct(AngVelDeg, Fwd); // ��/��

	// ������ PD ����: Torque = Kp * error - Kd * rate
	const float Control = (BankKp * Error) - (BankKd * RollRate);

	// �� �� ��ũ ����(�� ����)
	const FVector RollTorque = Fwd * Control;
	ShipMesh->AddTorqueInDegrees(RollTorque, NAME_None, /*bAccelChange=*/true);
}

void AMyTestPawn::ClampSpeeds() const
{
	// ���� ����
	const FVector V = ShipMesh->GetPhysicsLinearVelocity();
	const float Speed = V.Length();
	if (Speed > MaxLinearSpeed)
	{
		const FVector Clamped = V.GetSafeNormal() * MaxLinearSpeed;
		ShipMesh->SetPhysicsLinearVelocity(Clamped);
	}

	// ���� ����(��/��)
	const FVector AV = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const float AVMag = AV.Length();
	if (AVMag > MaxAngularSpeed)
	{
		const FVector ClampedAV = AV.GetSafeNormal() * MaxAngularSpeed;
		ShipMesh->SetPhysicsAngularVelocityInDegrees(ClampedAV);
	}
}

void AMyTestPawn::ApplyUpright(float DeltaTime, float Strength)
{
	if (Strength <= KINDA_SMALL_NUMBER)
		return;

	// ���ϴ� Up = ���� Up
	const FVector DesiredUp = FVector::UpVector;
	const FVector CurrUp = GetActorUpVector();

	// ȸ����/����
	FVector Axis = FVector::CrossProduct(CurrUp, DesiredUp);
	const float SinTheta = Axis.Length();
	const float CosTheta = FVector::DotProduct(CurrUp, DesiredUp);

	if (SinTheta < 1e-3f && CosTheta > 0.999f)
		return;

	Axis.Normalize();

	const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(SinTheta, CosTheta));

	// ���� ���ӵ����� ������ ���и�
	const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const float   RateAlongAxis = FVector::DotProduct(AngVelDeg, Axis);

	// PD + ���� ������
	const float Control = Strength * ((UprightKp * AngleDeg) - (UprightKd * RateAlongAxis));
	const FVector UprightTorque = Axis * Control;
	ShipMesh->AddTorqueInDegrees(UprightTorque, NAME_None, /*bAccelChange=*/true);

	// (�ɼ�) ���� Yaw�� ���͵� ���� ������ ����
	if (bRestoreYawToInitial)
	{
		const float CurrYaw = GetActorRotation().Yaw;
		const float YawErr = FMath::FindDeltaAngleDegrees(CurrYaw, InitialYaw);

		const float YawRate = FVector::DotProduct(AngVelDeg, FVector::UpVector);

		const float YawControl = Strength * ((YawKp * YawErr) - (YawKd * YawRate));
		const FVector YawTorque = FVector::UpVector * YawControl;

		ShipMesh->AddTorqueInDegrees(YawTorque, NAME_None, /*bAccelChange=*/true);
	}
}