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

void UMyShipMovement::BeginPlay()
{
	Super::BeginPlay();
}

void UMyShipMovement::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!ShipMesh || !GetOwner())
	{
		return;
	}

	ApplyForces(DeltaTime);
	ApplyTorques(DeltaTime);
	ClampSpeeds();
}


void UMyShipMovement::MoveForward(const FInputActionValue& Value)
{
	const float In = Value.Get<float>();
	ThrottleInput = FMath::Clamp(In, 0.f, 1.f);

}

void UMyShipMovement::Look(const FInputActionValue& Value)
{
	if (!ShipMesh || !GetOwner()) return;

	const FVector2D Axis = Value.Get<FVector2D>();
	CurrentLookInput.X = FMath::Clamp(Axis.X, -1.f, 1.f);
	CurrentLookInput.Y = FMath::Clamp(Axis.Y, -1.f, 1.f) * (bInvertPitch ? -1.f : 1.f);
}

void UMyShipMovement::LookEnded(const FInputActionValue& /*Value*/)
{
	CurrentLookInput = FVector2D::ZeroVector;
}

void UMyShipMovement::StartBoost() { bIsBoosting = true; }
void UMyShipMovement::StopBoost() { bIsBoosting = false; }

void UMyShipMovement::StartBrake() { bIsBraking = true; }
void UMyShipMovement::StopBrake() { bIsBraking = false; }

// ===== 내부 제어 =====

void UMyShipMovement::Roll(const FInputActionValue& Value)
{
	RollInput = FMath::Clamp(Value.Get<float>(), -1.f, 1.f);
}

void UMyShipMovement::ApplyForces(float DeltaTime)
{
	if (!ShipMesh) return;

	// --- 브레이크 우선 ---
	if (bIsBraking)
	{
		// 현재 속도를 BrakeDecel로 감속. 0 아래로는 떨어지지 않게 클램프.
		const FVector V = ShipMesh->GetPhysicsLinearVelocity();
		const float   S = V.Size();

		if (S > KINDA_SMALL_NUMBER)
		{
			const float NewS = FMath::Max(0.f, S - BrakeDecel * DeltaTime);
			const FVector NewV = (NewS > 0.f) ? (V * (NewS / S)) : FVector::ZeroVector;
			ShipMesh->SetPhysicsLinearVelocity(NewV, false);
		}

		// 브레이크 중에는 추력 무시(감속 우선)
		return;
	}

	// --- 전진 추력 ---
	if (ThrottleInput > 0.f)
	{
		const float Boost = bIsBoosting ? BoostMultiplier : 1.f;
		const float Force = ThrustForce * ThrottleInput * Boost;

		if (!FMath::IsNearlyZero(Force))
		{
			const FVector Fwd = ShipMesh->GetForwardVector();
			ShipMesh->AddForce(Fwd * Force, NAME_None, true /* AccelChange */);
		}
	}
}

void UMyShipMovement::ApplyTorques(float /*DeltaTime*/)
{
	if (!ShipMesh) return;

	// 기준 축
	const FVector Fwd = ShipMesh->GetForwardVector(); // 롤
	const FVector Up = ShipMesh->GetUpVector();      // 요
	const FVector Right = ShipMesh->GetRightVector();   // 피치

	// 마우스: 피치/요만 (롤은 절대 주지 않음)
	const float YawTorque = TurnTorque * CurrentLookInput.X;
	const float PitchTorque = -TurnTorque * CurrentLookInput.Y;

	if (!FMath::IsNearlyZero(YawTorque))
	{
		ShipMesh->AddTorqueInDegrees(Up * YawTorque, NAME_None, true);
	}

	if (!FMath::IsNearlyZero(PitchTorque))
	{
		ShipMesh->AddTorqueInDegrees(Right * PitchTorque, NAME_None, true);
	}

	// A/D: 롤만
	const float RollTorque = ManualRollTorque * RollInput;
	if (!FMath::IsNearlyZero(RollTorque))
	{
		ShipMesh->AddTorqueInDegrees(Fwd * RollTorque, NAME_None, true);
	}

}


void UMyShipMovement::ClampSpeeds() const
{
	if (!ShipMesh) return;

	const FVector V = ShipMesh->GetPhysicsLinearVelocity();
	const FVector Fwd = ShipMesh->GetForwardVector();

	const float Vforward = FVector::DotProduct(V, Fwd);
	const bool  bAccel = (ThrottleInput > 0.f || bIsBoosting) && !bIsBraking;

	// 너무 잦은 토글/부동소수 떨림 방지를 위한 여유값
	constexpr float Eps = 1.0f; // 1 cm/s

	// ── 전방 성분만 상한 클램프 ─────────────────────────────
	if (bAccel && Vforward > MaxLinearSpeed + Eps)
	{
		const FVector Vside = V - (Vforward * Fwd);
		const FVector ClampedForward = Fwd * MaxLinearSpeed;
		ShipMesh->SetPhysicsLinearVelocity(Vside + ClampedForward, false);
	}

	// ── 각속도도 "넘었을 때만" 제한 ─────────────────────────
	const FVector AV = ShipMesh->GetPhysicsAngularVelocityInDegrees();
	const float   AVsz = AV.Size();
	if (AVsz > MaxAngularSpeed + 0.1f)
	{
		ShipMesh->SetPhysicsAngularVelocityInDegrees(AV.GetSafeNormal() * MaxAngularSpeed, false);
	}
}
