// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"

class UInputMappingContext;
class AMyTestPawn;
class ASpaceCharacter;
//class AMySpaceShip;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerPawnChanged, APawn*, NewPawn);

UCLASS()
class GAMEENGINEBASIC_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	// IMC for character
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CharacterInputContext;

	// IMC for vehicle
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* ShipInputContext;


	TObjectPtr<AMyTestPawn> VehiclePawn;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float Delta) override;

public:
	UPROPERTY(BlueprintAssignable, Category = "Event")
	FOnPlayerPawnChanged OnPlayerPawnChanged;
};
