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
class UMyShipMovement;
class UShooterComp;

UCLASS()
class GAMEENGINEBASIC_API AMyTestPawn : public APawn
{
	GENERATED_BODY()

public:
	AMyTestPawn();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShipMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UMyShipMovement* ShipMovement;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UShooterComp* Shooter;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_MoveForward;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Roll;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Boost;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Fire;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Brake;

protected:
	// 게임 시작 시 호출되는 함수
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void MoveForward(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);

	void Boost_Pressed();
	void Boost_Released();

	void Brake_Pressed();
	void Brake_Released();

	void FireTriggered(const FInputActionValue& Value);   // 연사
	void FireStarted(const FInputActionValue& Value);     // 단발
	void FireCompleted(const FInputActionValue& Value);   
};
