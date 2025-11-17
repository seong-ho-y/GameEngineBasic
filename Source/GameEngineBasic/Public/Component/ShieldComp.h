// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShieldComp.generated.h"


class UHealthComp;
class AShieldActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldActivated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShieldDeactivated);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UShieldComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UShieldComp();

	UPROPERTY()
	AShieldActor* ShieldActor;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AShieldActor> ShieldActorClass;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    UHealthComp* HealthComp;

	bool bShieldActive = false;

public:
	// 스킬 사용으로 생성되는 쉴드량
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shield Ability")
	int32 ShieldAmount = 5;

public:
	UFUNCTION(BlueprintCallable)
	void ActivateShield();

	UFUNCTION(BlueprintCallable)
	void DeactivateShield();

	// Delegates
	UPROPERTY(BlueprintAssignable)
	FOnShieldActivated OnShieldActivated;

	UPROPERTY(BlueprintAssignable)
	FOnShieldDeactivated OnShieldDeactivated;

private:
	UFUNCTION()
	void OnShieldBrokenHandler(AActor* Owner);
};
