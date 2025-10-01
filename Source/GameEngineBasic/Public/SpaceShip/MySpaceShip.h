#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "MySpaceShip.generated.h"

// 전방 선언
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class GAMEENGINEBASIC_API AMySpaceShip : public APawn
{
	GENERATED_BODY()

public:
	AMySpaceShip();

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
	UInputAction* IA_MoveUpDown;

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
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float ThrustForce = 300.0f; // 전진 추력

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float UpDownForce = 150.0f; // 상하 이동 힘

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float TurnTorque = 300.0f; // 회전 토크

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float BoostMultiplier = 2.0f; // 부스트 시 추력 배율

	UPROPERTY(EditAnywhere, Category = "Physics")
	float RollSpeed = 5.0f; // 기울기가 얼마나 빨리 적용될지

	UPROPERTY(EditAnywhere, Category = "Physics")
	float MaxRollAngle = 30.0f; // 최대 기울기 각도

private:
	// 부스트 여부
	bool bIsBoosting = false;

	// 현재 마우스 입력을 저장할 변수
	FVector2D CurrentLookInput;

	// 입력 처리 함수들
	void MoveForward(const FInputActionValue& Value);
	void MoveUpDown(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Boost();
};
