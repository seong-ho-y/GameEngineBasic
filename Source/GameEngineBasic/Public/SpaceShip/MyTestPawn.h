#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "MyTestPawn.generated.h"

// ���� ����
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
	UInputAction* IA_Look; // ���콺 �Է��� ���� 2D �׼�

protected:
	// ���� ���� �� ȣ��Ǵ� �Լ�
	virtual void BeginPlay() override;

public:
	// �� �����Ӹ��� ȣ��Ǵ� �Լ�
	virtual void Tick(float DeltaTime) override;

	// �Է¿� ���� �̵� ó��
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// ���� ��ũ�� ���⸦ �����ϴ� ������
	UPROPERTY(EditAnywhere, Category = "Physics");
	float ThrustForce = 1000.0f; // ���� �߷�

	UPROPERTY(EditAnywhere, Category = "Physics");
	float TurnTorque = 300.0f; // ȸ�� ��ũ

	UPROPERTY(EditAnywhere, Category = "Physics");
	float BoostMultiplier = 2.0f; // �ν�Ʈ �� �߷� ����

	UPROPERTY(EditAnywhere, Category = "Physics")
	float RollSpeed = 5.0f; // ���Ⱑ �󸶳� ���� �������

	// ������ ȸ�� ����ȭ�� ���� PD ����(��ŷ)
	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0"))
	float BankKp = 6.0f;                 // ��ǥ �� ������ ���� ��� �̵�

	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0"))
	float BankKd = 1.5f;                 // �� ���ӵ� ���� �̵�

	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0", ClampMax = "89"))
	float MaxRollAngle = 30.0f;          // �ִ� ��(����) ����(��)

	// ���Ѱ�(�ʹ� Ƣ�� �� ����)
	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxLinearSpeed = 6000.0f;      // �ִ� ����(uu/s)

	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxAngularSpeed = 120.0f;      // �ִ� ����(��/��)

	// �ڵ� ������ ���� PD ����

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightKp = 8.0f;       // Up ���� ��� �̵�

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightKd = 2.5f;       // Up ���� ���� ���� �̵�

	UPROPERTY(EditAnywhere, Category = "Physics|Upright")
	bool bRestoreYawToInitial = false; // true�� ���� ������ε� õõ�� ����

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (EditCondition = "bRestoreYawToInitial", ClampMin = "0"))
	float YawKp = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (EditCondition = "bRestoreYawToInitial", ClampMin = "0"))
	float YawKd = 0.8f;

	// === Upright ���� ���� ===
	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0", ClampMax = "1"))
	float UprightMaxStrength = 1.0f;     // Upright �ִ� ���� ����

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightBlendInSpeed = 10.0f;    // �Է��� ������� �� ������ �ӵ�

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightBlendOutSpeed = 2.0f;   // �Է��� ���� �� ������ �ӵ�

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0", ClampMax = "1"))
	float LookDeadzone = 0.08f;          // �� �� ���� �Է��� '����'���� ����

private:

	float UprightAlpha = 1.0f; // ���� Upright ����(0~1)

	float InitialYaw = 0.f; // ���� ��� ����

	bool bIsBoosting = false; // �ν�Ʈ ����

	// ���� ���콺 �Է��� ������ ����
	FVector2D CurrentLookInput;

	// �Է� ó�� �Լ���
	void MoveForward(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void LookEnded(const FInputActionValue& Value); // �Է� ������ ��
	void Boost();


	// ���ѱ� ����
	void ApplyUpright(float DeltaTime, float Strength);

	// ���� ����
	void ApplyBankControl(float DeltaTime);  // ��(PD) ����
	void ClampSpeeds() const;                // �ӵ�/���ӵ� Ŭ����
};
