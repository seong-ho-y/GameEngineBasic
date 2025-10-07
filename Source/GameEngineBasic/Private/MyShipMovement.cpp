// Fill out your copyright notice in the Description page of Project Settings.


#include "MyShipMovement/MyShipMovement.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "InputActionValue.h"
#include "Engine/World.h"


// Sets default values for this component's properties
UMyShipMovement::UMyShipMovement()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UMyShipMovement::BeginPlay()
{
	Super::BeginPlay();

	// Initialize�� ȣ����� �ʾ��� ����� ������ġ
	if (!ShipMesh)
	{
		// �⺻������ ������ ��Ʈ�� UStaticMeshComponent��� �ڵ� ����
		if (AActor* Owner = GetOwner())
		{
			if (USceneComponent* Root = Owner->GetRootComponent())
			{
				ShipMesh = Cast<UStaticMeshComponent>(Root);
			}
			if (FMath::IsNearlyZero(InitialYaw))
			{
				InitialYaw = Owner->GetActorRotation().Yaw;
			}
		}
	}
}


// Called every frame
void UMyShipMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ShipMesh || !GetOwner())
	{
		return;
	}

	// Pawn.cpp�� Tick�� ������ ������ ����
	const float LookMag = CurrentLookInput.Length();
	const bool bHasLook = LookMag > LookDeadzone;

	const float TargetAlpha = bHasLook ? 0.0f : 1.0f;
	const float BlendSpeed = bHasLook ? UprightBlendOutSpeed : UprightBlendInSpeed;

	UprightAlpha = FMath::FInterpTo(UprightAlpha, TargetAlpha, DeltaTime, BlendSpeed);

	// Upright ���� (�ִ� ���� ���� ����)
	ApplyUpright(DeltaTime, UprightAlpha * UprightMaxStrength);

	// �� PD ���� + �ӵ� Ŭ����
	ApplyBankControl(DeltaTime);
	ClampSpeeds();
}

void UMyShipMovement::Initialize(UStaticMeshComponent* InShipMesh, float InInitialYaw)
{
	ShipMesh = InShipMesh;
	InitialYaw = InInitialYaw;
}


void UMyShipMovement::MoveForward(const FInputActionValue& Value)
{
	if (!ShipMesh || !GetOwner()) return;

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

	const FVector Forward = GetOwner()->GetActorForwardVector();
	ShipMesh->AddForce(Forward * CurrentThrust, NAME_None, /*bAccelChange=*/true);
}

void UMyShipMovement::Look(const FInputActionValue& Value)
{
	if (!ShipMesh || !GetOwner()) return;

	CurrentLookInput = Value.Get<FVector2D>();

	const FVector Right = GetOwner()->GetActorRightVector();
	const FVector Up = GetOwner()->GetActorUpVector();

	const FVector PitchTorque = Right * (-CurrentLookInput.Y * TurnTorque);
	const FVector YawTorque = Up * (CurrentLookInput.X * TurnTorque);

	ShipMesh->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, /*bAccelChange=*/true);
}

void UMyShipMovement::LookEnded(const FInputActionValue& /*Value*/)
{
	CurrentLookInput = FVector2D::ZeroVector;
}

void UMyShipMovement::Boost()
{
	bIsBoosting = !bIsBoosting;
}

// ===== ���� ���� =====

void UMyShipMovement::ApplyBankControl(float /*DeltaTime*/)
{
	if (!ShipMesh || !GetOwner()) return;

	// ���콺 X�� ����� ��ǥ �� ��(��)
	const float TargetRoll = FMath::Clamp(CurrentLookInput.X, -1.0f, 1.0f) * MaxRollAngle;

	// ���� ��(��)
	const float CurrentRoll = GetOwner()->GetActorRotation().Roll;

	// ����
	const float Error = FMath::FindDeltaAngleDegrees(CurrentRoll, TargetRoll);

	// ���� ���ӵ�(��/��)�� "�� �� ����" (���� = ������)
	const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const FVector Fwd = GetOwner()->GetActorForwardVector();
	const float RollRate = FVector::DotProduct(AngVelDeg, Fwd);

	// PD ����
	const float Control = (BankKp * Error) - (BankKd * RollRate);

	// �� �� ��ũ
	const FVector RollTorque = Fwd * Control;
	ShipMesh->AddTorqueInDegrees(RollTorque, NAME_None, /*bAccelChange=*/true);
}

void UMyShipMovement::ClampSpeeds() const
{
	if (!ShipMesh) return;

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

void UMyShipMovement::ApplyUpright(float /*DeltaTime*/, float Strength)
{
	if (Strength <= KINDA_SMALL_NUMBER || !ShipMesh || !GetOwner()) return;

	const FVector WorldUp = FVector::UpVector;
	const FVector Fwd = GetOwner()->GetActorForwardVector();

	// 1) ���� Up�� Fwd�� ������ ������� ���� �� �Ѹ� ����
	FVector DesiredUp = WorldUp - FVector::DotProduct(WorldUp, Fwd) * Fwd;
	const float Len = DesiredUp.Size();
	if (Len < 1e-3f)
	{
		// "��" ��ó ������
		return;
	}
	DesiredUp /= Len;

	// 2) ���� Up���� ���̸� ��/������ ���
	const FVector CurrUp = GetOwner()->GetActorUpVector();
	FVector Axis = FVector::CrossProduct(CurrUp, DesiredUp);
	const float SinTheta = Axis.Size();
	const float CosTheta = FVector::DotProduct(CurrUp, DesiredUp);
	if (SinTheta < 1e-3f && CosTheta > 0.999f) return;
	Axis /= (SinTheta + KINDA_SMALL_NUMBER);

	const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(SinTheta, CosTheta));

	// 3) ���� ���ӵ��� ������ ���и� ����
	const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const float RateAlongAxis = FVector::DotProduct(AngVelDeg, Axis);

	// 4) "��" ��ó �����ϸ�
	const float PoleScale = 1.f - FMath::Abs(FVector::DotProduct(Fwd, WorldUp)); // ���� 1, ���� 0

	// 5) PD ���� + ������
	const float Control = Strength * PoleScale * ((UprightKp * AngleDeg) - (UprightKd * RateAlongAxis));
	ShipMesh->AddTorqueInDegrees(Axis * Control, NAME_None, /*bAccelChange=*/true);

	// (�ɼ�) Yaw ����
	if (bRestoreYawToInitial && PoleScale > 0.1f)
	{
		const float CurrYaw = GetOwner()->GetActorRotation().Yaw;
		const float YawErr = FMath::FindDeltaAngleDegrees(CurrYaw, InitialYaw);
		const float YawRate = FVector::DotProduct(AngVelDeg, WorldUp);

		const float YawControl = Strength * PoleScale * ((YawKp * YawErr) - (YawKd * YawRate));
		ShipMesh->AddTorqueInDegrees(WorldUp * YawControl, NAME_None, /*bAccelChange=*/true);
	}
}