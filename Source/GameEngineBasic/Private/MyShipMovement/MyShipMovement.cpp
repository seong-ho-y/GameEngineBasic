// Fill out your copyright notice in the Description page of Project Settings.


#include "MyShipMovement/MyShipMovement.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "InputActionValue.h"

#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/World.h"



// Sets default values for this component's properties
UMyShipMovement::UMyShipMovement()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMyShipMovement::BeginPlay()
{
	Super::BeginPlay();

	if (!ShipMesh)
	{
		if (AActor* Owner = GetOwner())
		{
			ShipMesh = Owner->FindComponentByClass<UStaticMeshComponent>();
		}
	}
	
	if (ShipMesh->DoesSocketExist(LeftThrusterSocket))
	{
		LeftThrusterComp = UGameplayStatics::SpawnEmitterAttached(
			ThrusterFX,
			ShipMesh,
			LeftThrusterSocket,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false);

		LeftThrusterComp->bAutoActivate = false;
		LeftThrusterComp->DeactivateSystem();
	}

	if (ShipMesh->DoesSocketExist(RightThrusterSocket))
	{
		RightThrusterComp = UGameplayStatics::SpawnEmitterAttached(
			ThrusterFX,
			ShipMesh,
			RightThrusterSocket,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false);

		RightThrusterComp->bAutoActivate = false;
		RightThrusterComp->DeactivateSystem();
	}

	if (ShipMesh->DoesSocketExist(MiddleThrusterSocket))
	{
		MiddleThrusterComp = UGameplayStatics::SpawnEmitterAttached(
			ThrusterFX,
			ShipMesh,
			MiddleThrusterSocket,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			false);

		MiddleThrusterComp->bAutoActivate = false;
		MiddleThrusterComp->DeactivateSystem();
	}
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

void UMyShipMovement::Initialize(UStaticMeshComponent* InShipMesh)
{
	ShipMesh = InShipMesh;
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

void UMyShipMovement::StartBoost()
{ 
	bIsBoosting = true; 

	if (LeftThrusterComp)  LeftThrusterComp->ActivateSystem(true);
	if (MiddleThrusterComp)  MiddleThrusterComp->ActivateSystem(true);
	if (RightThrusterComp) RightThrusterComp->ActivateSystem(true);
	
}
void UMyShipMovement::StopBoost() 
{ 
	bIsBoosting = false; 

	if (LeftThrusterComp)  LeftThrusterComp->DeactivateSystem();
	if (MiddleThrusterComp)  MiddleThrusterComp->DeactivateSystem();
	if (RightThrusterComp) RightThrusterComp->DeactivateSystem();
}

void UMyShipMovement::StartBrake() 
{ 
	bIsBraking = true; 
}
void UMyShipMovement::StopBrake() 
{ 
	bIsBraking = false;

}

// ===== 내부 제어 =====

void UMyShipMovement::Roll(const FInputActionValue& Value)
{
	RollInput = FMath::Clamp(Value.Get<float>(), -1.f, 1.f);
}

void UMyShipMovement::ApplyBrake(float DeltaTime)
{
	if (!ShipMesh) return;

	const FVector V = ShipMesh->GetPhysicsLinearVelocity();
	const float   S = V.Size();
	if (S <= KINDA_SMALL_NUMBER)
	{
		// 이미 거의 정지
		ShipMesh->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
		return;
	}

	// 목표 감속량 (이번 프레임에서 줄일 속도 크기)
	const float MaxDeltaSpeed = BrakeDecel * DeltaTime;

	// 1) 아주 느릴 때는 멈춤으로 스냅 (뒤집힘 방지)
	if (S <= MaxDeltaSpeed)
	{
		ShipMesh->SetPhysicsLinearVelocity(FVector::ZeroVector, false);
		return;
	}

	// 2) 정상 구간: F = m*a로 ‘현재 속도의 정반대’ 방향으로 일정 감속
	const float Mass = ShipMesh->GetMass();
	const FVector DirOpposite = -V.GetSafeNormal();             // 진행 반대
	const FVector Force = DirOpposite * (Mass * BrakeDecel);    // 실제 힘

	// 물리력으로 감속 (AccelChange=false: 진짜 Force로 적용, 질량 영향 O)
	ShipMesh->AddForce(Force, NAME_None, false);
}

void UMyShipMovement::ApplyForces(float DeltaTime)
{
	if (!ShipMesh) return;

	// 2) 브레이크 우선
	if (bIsBraking)
	{
		ApplyBrake(DeltaTime);
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
