#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "MyTestPawn.generated.h"

// 전방 선언
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class GAMEENGINEBASIC_API AMyTestPawn : public APawn
{
	GENERATED_BODY()

public:
	AMyTestPawn();

protected:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShipMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_MoveForward;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Boost;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look; // 마우스 입력을 위한 2D 액션

protected:
	// 게임 시작 시 호출되는 함수
	virtual void BeginPlay() override;

public:
	// 매 프레임마다 호출되는 함수
	virtual void Tick(float DeltaTime) override;

	// 입력에 따라 이동 처리
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// 힘과 토크의 세기를 조절하는 변수들
	UPROPERTY(EditAnywhere, Category = "Physics");
	float ThrustForce = 1000.0f; // 전진 추력

	UPROPERTY(EditAnywhere, Category = "Physics");
	float TurnTorque = 300.0f; // 회전 토크

	UPROPERTY(EditAnywhere, Category = "Physics");
	float BoostMultiplier = 2.0f; // 부스트 시 추력 배율

	UPROPERTY(EditAnywhere, Category = "Physics")
	float RollSpeed = 5.0f; // 기울기가 얼마나 빨리 적용될지

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
	float UprightKp = 8.0f;       // Up 정렬 비례 이득

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
	float UprightBlendInSpeed = 10.0f;    // 입력이 사라졌을 때 강도↑ 속도

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightBlendOutSpeed = 2.0f;   // 입력이 있을 때 강도↓ 속도

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0", ClampMax = "1"))
	float LookDeadzone = 0.08f;          // 이 값 이하 입력은 '없음'으로 간주

private:

	float UprightAlpha = 1.0f; // 현재 Upright 강도(0~1)

	float InitialYaw = 0.f; // 시작 헤딩 저장

	bool bIsBoosting = false; // 부스트 여부

	// 현재 마우스 입력을 저장할 변수
	FVector2D CurrentLookInput;

	// 입력 처리 함수들
	void MoveForward(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void LookEnded(const FInputActionValue& Value); // 입력 끝났을 때
	void Boost();


	// 오뚜기 복원
	void ApplyUpright(float DeltaTime, float Strength);

	// 내부 헬퍼
	void ApplyBankControl(float DeltaTime);  // 롤(PD) 제어
	void ClampSpeeds() const;                // 속도/각속도 클램프
};
