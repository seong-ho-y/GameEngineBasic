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

	// Pawn???? ????? ??? ???? (BeginPlay ???? ???)
	void Initialize(UStaticMeshComponent* InShipMesh, float InInitialYaw);

	// ??? ?????
	void MoveForward(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	void LookEnded(const struct FInputActionValue& Value);
	void Boost();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// --- ??? ??? ---
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


	// ?????? ??? ??????? ???? PD ????(???)
	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0"))
	float BankKp = 6.0f;                 // ??? ?? ?????? ???? ??? ???

	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0"))
	float BankKd = 1.5f;                 // ?? ????? ???? ???

	UPROPERTY(EditAnywhere, Category = "Physics|Bank", meta = (ClampMin = "0", ClampMax = "89"))
	float MaxRollAngle = 30.0f;          // ??? ??(????) ????(??)


	// ?????(??? ??? ?? ????)
	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxLinearSpeed = 6000.0f;      // ??? ????(uu/s)

	UPROPERTY(EditAnywhere, Category = "Limits", meta = (ClampMin = "0"))
	float MaxAngularSpeed = 120.0f;      // ??? ????(??/??)


	// ??? ?????? ???? PD ????
	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightKp = 25.0f;       // Up ???? ??? ???

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightKd = 2.5f;       // Up ???? ???? ???? ???

	UPROPERTY(EditAnywhere, Category = "Physics|Upright")
	bool bRestoreYawToInitial = false; // true?? ???? ???????? õõ?? ????

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (EditCondition = "bRestoreYawToInitial", ClampMin = "0"))
	float YawKp = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (EditCondition = "bRestoreYawToInitial", ClampMin = "0"))
	float YawKd = 0.8f;


	// === Upright ???? ???? ===
	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0", ClampMax = "1"))
	float UprightMaxStrength = 1.0f;     // Upright ??? ???? ????

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightBlendInSpeed = 30.0f;    // ????? ??????? ?? ?????? ???

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0"))
	float UprightBlendOutSpeed = 2.0f;   // ????? ???? ?? ?????? ???

	UPROPERTY(EditAnywhere, Category = "Physics|Upright", meta = (ClampMin = "0", ClampMax = "1"))
	float LookDeadzone = 0.08f;          // ?? ?? ???? ????? '????'???? ????


protected:
	// --- ???? ???? ---
	float UprightAlpha = 1.0f; // ???? Upright ????(0~1)
	float InitialYaw = 0.f; // ???? ??? ????
	bool bIsBoosting = false;
	FVector2D CurrentLookInput = FVector2D::ZeroVector; // ???? ???? ????? ?????? ????

	
	
	// ????? ????
	void ApplyUpright(float DeltaTime, float Strength);

	// --- ???? ???? ---
	void ApplyBankControl(float DeltaTime); // ??(PD) ????
	void ClampSpeeds() const; // ???/????? ?????
};
