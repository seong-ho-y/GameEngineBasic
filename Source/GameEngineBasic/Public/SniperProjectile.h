// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "SniperProjectile.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API ASniperProjectile : public AProjectile
{
	GENERATED_BODY()

public:
	ASniperProjectile();
	
	UFUNCTION()
	void HandleSniperHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sniper|Knockback")
	float KnockbackStrength = 2500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sniper|Knockback")
	float KnockbackUpStrength = 200.f;
	
};
