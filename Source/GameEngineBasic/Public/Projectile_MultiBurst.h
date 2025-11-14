// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Projectile_MultiBurst.generated.h"

UCLASS()
class GAMEENGINEBASIC_API AProjectile_MultiBurst : public AProjectile
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "MultiBurst")
	int32 NumShots = 3;

	UPROPERTY(EditAnywhere, Category = "MultiBurst")
	float Interval = 0.1f;

private:
	void SpawnSubProjectile();
	int32 CurrentShot = 0;

};
