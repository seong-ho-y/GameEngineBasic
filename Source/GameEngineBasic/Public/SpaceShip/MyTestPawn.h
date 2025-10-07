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
	UMyShipMovement* ShipMovement;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_MoveForward;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Boost;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look;

protected:
	// 게임 시작 시 호출되는 함수
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void MoveForward(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void LookEnded(const FInputActionValue& Value);
	void Boost();
};
