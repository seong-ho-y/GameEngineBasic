// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyShipMovement.generated.h"


class UStaticMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UMyShipMovement : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyShipMovement();

	// Pawn에서 메쉬를 넘겨 초기화 (BeginPlay 등에서 호출)
	void Initialize(UStaticMeshComponent* InShipMesh, float InInitialYaw);

	// 입력 전달용
	void MoveForward(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void LookEnded(const struct FInputActionValue& Value);
	void Boost();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// --- 대상 메쉬 ---
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShipMesh = nullptr;

	// --- Physics ---
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float ThrustForce = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float TurnTorque = 300.f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float BoostMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (ClampMin = "0"))
	float RollSpeed = 5.0f;


	// 은근한 회전 안정화를 위한 PD 제어(뱅킹)
	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0"))
	float BankKp = 6.0f;                 // 목표 롤 각도에 대한 비례 이득

	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0"))
	float BankKd = 1.5f;                 // 롤 각속도 감쇠 이득

	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0", ClampMax = "89"))
	float MaxRollAngle = 30.0f;          // 최대 롤(기울기) 각도(도)


	// 상한값(너무 튀는 것 방지)
	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxLinearSpeed = 6000.0f;      // 최대 선속(uu/s)

	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxAngularSpeed = 120.0f;      // 최대 각속(도/초)


	// 자동 정렬을 위한 PD 제어
	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightKp = 25.0f;       // Up 정렬 비례 이득

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightKd = 2.5f;       // Up 정렬 각속 감쇠 이득

	UPROPERTY(EditAnywhere, Category = "Physics|Upright")
	bool bRestoreYawToInitial = false; // true면 시작 헤딩으로도 천천히 복귀

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (EditCondition = "bRestoreYawToInitial", ClampMin = "0"))
	float YawKp = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (EditCondition = "bRestoreYawToInitial", ClampMin = "0"))
	float YawKd = 0.8f;


	// === Upright 블렌딩 설정 ===
	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0", ClampMax = "1"))
	float UprightMaxStrength = 1.0f;     // Upright 최대 강도 배율

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightBlendInSpeed = 30.0f;    // 입력이 사라졌을 때 강도↑ 속도

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightBlendOutSpeed = 2.0f;   // 입력이 있을 때 강도↓ 속도

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0", ClampMax = "1"))
	float LookDeadzone = 0.08f;          // 이 값 이하 입력은 '없음'으로 간주


protected:
	// --- 내부 상태 ---
	float UprightAlpha = 1.0f; // 현재 Upright 강도(0~1)
	float InitialYaw = 0.f; // 시작 헤딩 저장
	bool bIsBoosting = false;
	FVector2D CurrentLookInput = FVector2D::ZeroVector; // 현재 마우스 입력을 저장할 변수

	
	
	// 오뚜기 복원
	void ApplyUpright(float DeltaTime, float Strength);

	// --- 내부 헬퍼 ---
	void ApplyBankControl(float DeltaTime); // 롤(PD) 제어
	void ClampSpeeds() const; // 속도/각속도 클램프
};
