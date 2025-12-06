// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyHuman.h"
#include "TurretLaserComponent.h"
#include "EnemyTurret.generated.h"

/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API AEnemyTurret : public AEnemyHuman
{
	GENERATED_BODY()
public:
	AEnemyTurret();

	UPROPERTY(EditAnywhere, Category = "Component")
	UTurretLaserComponent* TurretLaserComp;

	bool bTurretStunned = false;
	bool bTurretDead = false;
	FTimerHandle TimerHandle_Stun;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	virtual void OnKnock() override;
	void EndStun();
	virtual void OnDie(AActor* DeadActor) override;

private:
	UPROPERTY()
	class UTurretAnimInstance* TurretAnim;

	UPROPERTY(EditAnywhere, Category = "Turret")
	FName HeadBoneName = "bn_head";

	// 회전 속도
	UPROPERTY(EditAnywhere, Category="Turret")
	float RotateSpeed = 6.f;

	void UpdateTurretHead(float DeltaSeconds);
	
};
