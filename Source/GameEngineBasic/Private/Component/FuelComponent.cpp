// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/FuelComponent.h"

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
    CurrentFuel = FMath::Clamp(CurrentFuel - Amount, 0.f, MaxFuel);
}

void UFuelComponent::ConsumeFlying(float DeltaTime)
{
    Consume(FlyingConsumeRate * DeltaTime);
}

void UFuelComponent::Recharge(float DeltaTime)
{
    CurrentFuel = FMath::Clamp(CurrentFuel + RechargeRate * DeltaTime, 0.f, MaxFuel);
}