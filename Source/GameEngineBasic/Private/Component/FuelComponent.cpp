// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/FuelComponent.h"
#include "SpaceCharacter/SpaceCharacter.h"

UFuelComponent::UFuelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UFuelComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UFuelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    GEngine->AddOnScreenDebugMessage(35, 5.f, FColor::Silver, FString::Printf(TEXT("Remain Energy : %f"),CurrentFuel));
    Recharge(DeltaTime);
}

bool UFuelComponent::HasFuel(float Amount) const
{
    return CurrentFuel >= Amount;
}

bool UFuelComponent::CanBoost() const
{
    return CurrentFuel >= BoostCost;
}

bool UFuelComponent::CanFly() const
{
    return CurrentFuel > 5.f;
}

void UFuelComponent::Consume(float Amount)
{
    GEngine->AddOnScreenDebugMessage(33, 1, FColor::Orange, TEXT("Consume Called"));
    float OldFuel = CurrentFuel;
    CurrentFuel = FMath::Clamp(CurrentFuel - Amount, 0.f, MaxFuel);
    /*
    if(GEngine)
        GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Fuel: %.2f / %.2f"), CurrentFuel, MaxFuel));
    */

    if (CurrentFuel!=OldFuel)
        OnFuelChanged.Broadcast(CurrentFuel,MaxFuel);
}

void UFuelComponent::ConsumeFlying(float DeltaTime)
{
    Consume(FlyingConsumeRate * DeltaTime);
}

void UFuelComponent::Recharge(float DeltaTime)
{
	auto Character = Cast<ASpaceCharacter>(GetOwner());
    if (Character->bIsSprinting)
        return;

    float OldFuel = CurrentFuel;
    CurrentFuel = FMath::Clamp(CurrentFuel + RechargeRate * DeltaTime, 0.f, MaxFuel);
    if (CurrentFuel != OldFuel)
        OnFuelChanged.Broadcast(CurrentFuel, MaxFuel);
    
}