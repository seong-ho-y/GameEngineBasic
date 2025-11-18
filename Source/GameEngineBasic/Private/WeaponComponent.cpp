// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::InitializeWeapon(ASpaceCharacter* Player, UShooterComp* InShooterComp)
{
	OwnerCharacter = Player;
	ShooterComp = InShooterComp;
}


FVector UWeaponComponent::GetAimDirection() const
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponComp : No Player Found!"));
		return FVector::ForwardVector;
	}
	if (!ShooterComp) return FVector::ForwardVector;
	FVector AimPoint = GetAimPoint();
	FVector MuzzleLoc = GetMuzzleLoc();

	return (AimPoint - MuzzleLoc).GetSafeNormal();
}

void UWeaponComponent::HandleFirePressed()
{
	PerformFire();
}

void UWeaponComponent::HandleFireReleased()
{
	
}

bool UWeaponComponent::CanFire() const
{
	return true;
}

void UWeaponComponent::PerformFire()
{
	ShooterComp->SetFireDirection(GetAimDirection());
}

FVector UWeaponComponent::GetAimPoint() const
{
	
}
FVector UWeaponComponent::GetMuzzleLoc() const
{
	
}