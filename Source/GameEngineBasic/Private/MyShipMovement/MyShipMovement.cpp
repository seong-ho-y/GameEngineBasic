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

	// Initialize가 호출되지 않았을 경우의 안전장치
	if (!ShipMesh)
	{
		// 기본적으로 오너의 루트가 UStaticMeshComponent라면 자동 추정
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

	// Pawn.cpp의 Tick과 동일한 블렌딩 로직
	const float LookMag = CurrentLookInput.Length();
	const bool bHasLook = LookMag > LookDeadzone;

	const float TargetAlpha = bHasLook ? 0.0f : 1.0f;
	const float BlendSpeed = bHasLook ? UprightBlendOutSpeed : UprightBlendInSpeed;

	UprightAlpha = FMath::FInterpTo(UprightAlpha, TargetAlpha, DeltaTime, BlendSpeed);

	// Upright 적용 (최대 강도 배율 포함)
	ApplyUpright(DeltaTime, UprightAlpha * UprightMaxStrength);

	// 롤 PD 제어 + 속도 클램프
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

// ===== 내부 제어 =====

void UMyShipMovement::ApplyBankControl(float /*DeltaTime*/)
{
	if (!ShipMesh || !GetOwner()) return;

	// 마우스 X에 비례한 목표 롤 각(도)
	const float TargetRoll = FMath::Clamp(CurrentLookInput.X, -1.0f, 1.0f) * MaxRollAngle;

	// 현재 롤(도)
	const float CurrentRoll = GetOwner()->GetActorRotation().Roll;

	// 오차
	const float Error = FMath::FindDeltaAngleDegrees(CurrentRoll, TargetRoll);

	// 현재 각속도(도/초)의 "롤 축 성분" (롤축 = 포워드)
	const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const FVector Fwd = GetOwner()->GetActorForwardVector();
	const float RollRate = FVector::DotProduct(AngVelDeg, Fwd);

	// PD 제어
	const float Control = (BankKp * Error) - (BankKd * RollRate);

	// 롤 축 토크
	const FVector RollTorque = Fwd * Control;
	ShipMesh->AddTorqueInDegrees(RollTorque, NAME_None, /*bAccelChange=*/true);
}

void UMyShipMovement::ClampSpeeds() const
{
	if (!ShipMesh) return;

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

void UMyShipMovement::ApplyUpright(float /*DeltaTime*/, float Strength)
{
	if (Strength <= KINDA_SMALL_NUMBER || !ShipMesh || !GetOwner()) return;

	const FVector WorldUp = FVector::UpVector;
	const FVector Fwd = GetOwner()->GetActorForwardVector();

	// 1) 월드 Up을 Fwd에 직교한 평면으로 투영 → 롤만 교정
	FVector DesiredUp = WorldUp - FVector::DotProduct(WorldUp, Fwd) * Fwd;
	const float Len = DesiredUp.Size();
	if (Len < 1e-3f)
	{
		// "극" 근처 안정성
		return;
	}
	DesiredUp /= Len;

	// 2) 현재 Up과의 차이를 축/각도로 계산
	const FVector CurrUp = GetOwner()->GetActorUpVector();
	FVector Axis = FVector::CrossProduct(CurrUp, DesiredUp);
	const float SinTheta = Axis.Size();
	const float CosTheta = FVector::DotProduct(CurrUp, DesiredUp);
	if (SinTheta < 1e-3f && CosTheta > 0.999f) return;
	Axis /= (SinTheta + KINDA_SMALL_NUMBER);

	const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(SinTheta, CosTheta));

	// 3) 현재 각속도의 교정축 성분만 감쇠
	const FVector AngVelDeg = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const float RateAlongAxis = FVector::DotProduct(AngVelDeg, Axis);

	// 4) "극" 근처 스케일링
	const float PoleScale = 1.f - FMath::Abs(FVector::DotProduct(Fwd, WorldUp)); // 수평 1, 수직 0

	// 5) PD 제어 + 블렌드
	const float Control = Strength * PoleScale * ((UprightKp * AngleDeg) - (UprightKd * RateAlongAxis));
	ShipMesh->AddTorqueInDegrees(Axis * Control, NAME_None, /*bAccelChange=*/true);

	// (옵션) Yaw 복원
	if (bRestoreYawToInitial && PoleScale > 0.1f)
	{
		const float CurrYaw = GetOwner()->GetActorRotation().Yaw;
		const float YawErr = FMath::FindDeltaAngleDegrees(CurrYaw, InitialYaw);
		const float YawRate = FVector::DotProduct(AngVelDeg, WorldUp);

		const float YawControl = Strength * PoleScale * ((YawKp * YawErr) - (YawKd * YawRate));
		ShipMesh->AddTorqueInDegrees(WorldUp * YawControl, NAME_None, /*bAccelChange=*/true);
	}
}