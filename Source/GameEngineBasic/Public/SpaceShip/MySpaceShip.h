#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "MySpaceShip.generated.h"

// ���� ����
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
	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float ThrustForce = 300.0f; // ���� �߷�

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float UpDownForce = 150.0f; // ���� �̵� ��

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float TurnTorque = 300.0f; // ȸ�� ��ũ

	UPROPERTY(EditAnywhere, Category = "Physics", meta = (META_DATA_SPECIFIER));
	float BoostMultiplier = 2.0f; // �ν�Ʈ �� �߷� ����

	UPROPERTY(EditAnywhere, Category = "Physics")
	float RollSpeed = 5.0f; // ���Ⱑ �󸶳� ���� �������

	UPROPERTY(EditAnywhere, Category = "Physics")
	float MaxRollAngle = 30.0f; // �ִ� ���� ����

private:
	// �ν�Ʈ ����
	bool bIsBoosting = false;

	// ���� ���콺 �Է��� ������ ����
	FVector2D CurrentLookInput;

	// �Է� ó�� �Լ���
	void MoveForward(const FInputActionValue& Value);
	void MoveUpDown(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Boost();
};
