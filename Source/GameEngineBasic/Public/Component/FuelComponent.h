// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FuelComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FFuelChanged, float, CurrentFuel, float, MaxFuel);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UFuelComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UFuelComponent();
    
    UPROPERTY(BlueprintAssignable)
    FFuelChanged OnFuelChanged;
protected:
    virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, Category = "Fuel")
    float MaxFuel = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "Fuel")
    float CurrentFuel = 100.f;

    UPROPERTY(EditAnywhere, Category = "Fuel|Rates")
    float FlyingConsumeRate = 12.f;

    UPROPERTY(EditAnywhere, Category = "Fuel|Rates")
    float RechargeRate = 10.f;

    UPROPERTY(EditAnywhere, Category = "Fuel|Boost")
    float BoostCost = 20.f;

    UFUNCTION(BlueprintCallable, Category = "Flight|Fuel")
    float GetFuelPercent() const {
        return (MaxFuel > 0) ? CurrentFuel / MaxFuel : 0.f;
    }
    

public:
    float GetFuel() const { return CurrentFuel; }
    float GetMaxFuel() const { return MaxFuel; }

    bool HasFuel(float Amount = 1.f) const;

    bool CanBoost() const;
    bool CanFly() const;

    void Consume(float Amount);
    void ConsumeFlying(float DeltaTime);
    void ConsumeDash(float Amount);
    void Recharge(float DeltaTime);

    float GetBoostCost() const { return BoostCost; }
};