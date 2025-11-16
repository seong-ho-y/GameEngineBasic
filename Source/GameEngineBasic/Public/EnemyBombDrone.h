// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyDrone.h"
#include "EnemyBombDrone.generated.h"

UCLASS()
class GAMEENGINEBASIC_API AEnemyBombDrone : public AEnemyDrone
{
	GENERATED_BODY()


public:
	AEnemyBombDrone();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	UPROPERTY(EditAnywhere, Category="BombDrone")
	float ExplodeDistance = 200.f;   // 플레이어 반경 n미터 안이면 폭발

public:
	
	// 폭발할 때 이펙트/데미지 담당
	UPROPERTY(EditAnywhere, Category="Explosion")
	class UExplosionComponent* ExplosionComp;


private:
	AActor* Player;
};
