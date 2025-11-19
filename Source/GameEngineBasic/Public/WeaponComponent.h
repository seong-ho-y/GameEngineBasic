// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "WeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

	void InitializeWeapon(ASpaceCharacter* Player, UShooterComp* InShooterComp);


	virtual void HandleFirePressed();
	virtual void HandleFireReleased();

	virtual bool CanFire() const;
	virtual void PerformFire();
	FVector GetAimPoint() const;
	FVector GetMuzzleLoc() const;

protected:

	UPROPERTY()
	ASpaceCharacter* OwnerCharacter;

	UPROPERTY()
	UShooterComp* ShooterComp;

	
	UPROPERTY(EditAnywhere, Category = "Muzzle")
	FName MuzzleSocketName;
	
	virtual FVector GetAimDirection() const; // If there are more than one aim logic, you can override in child weapon
public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
