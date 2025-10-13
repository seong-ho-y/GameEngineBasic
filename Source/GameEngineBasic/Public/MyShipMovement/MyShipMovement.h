// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "MyShipMovement.generated.h"


class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UMyShipMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyShipMovement();

	// Pawn에서 메쉬를 넘겨 초기화 (BeginPlay 등에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Ship")
	void Initialize(UStaticMeshComponent* InShipMesh);

	// 입력 전달용
	void MoveForward(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void LookEnded(const struct FInputActionValue& Value);
	void Roll(const struct FInputActionValue& Value);
	void ApplyBrake(float DeltaTime);
	void StartBrake();
	void StopBrake();
	void StartBoost();
	void StopBoost();


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// --- 대상 메쉬 ---
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShipMesh = nullptr;

	UPROPERTY(EditAnywhere, Category = "FX|Thruster")
	UParticleSystem* ThrusterFX;      // Cascade 파티클 에셋

	UPROPERTY(EditAnywhere, Category = "FX|Thruster")
	FName LeftThrusterSocket = "Thruster_L";

	UPROPERTY(EditAnywhere, Category = "FX|Thruster")
	FName MiddleThrusterSocket = "Thruster_M";

	UPROPERTY(EditAnywhere, Category = "FX|Thruster")
	FName RightThrusterSocket = "Thruster_R";

	UPROPERTY(Transient) 
	UParticleSystemComponent* LeftThrusterComp = nullptr;

	UPROPERTY(Transient)
	UParticleSystemComponent* MiddleThrusterComp = nullptr;

	UPROPERTY(Transient)
	UParticleSystemComponent* RightThrusterComp = nullptr;

	// --- Physics ---
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float ThrustForce = 1500.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float TurnTorque = 300.f; // 클수록 마우스 감도 증가

	UPROPERTY(EditAnywhere, Category = "Physics|Linear", meta = (ClampMin = "0"))
	float BrakeDecel = 200.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float BoostMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float RollSpeed = 5.0f;

	// 롤 토크(A/D 전용)
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float ManualRollTorque = 180.f;

	// 마우스 피치 반전 여부
	UPROPERTY(EditAnywhere, Category = "Input")
	bool bInvertPitch = false;

	// 상한값(너무 튀는 것 방지)
	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxLinearSpeed = 6000.0f;      // 최대 선속(uu/s)

	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxAngularSpeed = 120.0f;      // 최대 각속(도/초)

protected:

	// W/S
	float ThrottleInput = 0.f;          // -1..1
	// 마우스 XY (Yaw=X, Pitch=Y)
	FVector2D CurrentLookInput = FVector2D::ZeroVector; // -1..1
	// A/D
	float RollInput = 0.f;              // -1..1
	// 부스트
	bool bIsBoosting = false;
	bool bIsBraking = false;
	
	void ApplyForces(float DeltaTime);
	void ApplyTorques(float DeltaTime); 
	void ClampSpeeds() const; // 속도/각속도 클램프
};
