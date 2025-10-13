// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComp.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UHealthComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHealthComp();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Health")
	float MaxHealth;

	float CurrentHealth;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Health")
	float MaxShield;

	float CurrentShield;

	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage();

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal();
	
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitStats();
};
