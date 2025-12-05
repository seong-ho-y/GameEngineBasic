#pragma once
#include "CoreMinimal.h"
#include "DroneMovementComponent.h"
#include "EnemyHuman.h"
#include "EnemyDrone.generated.h"

UCLASS()
class GAMEENGINEBASIC_API AEnemyDrone : public AEnemyHuman
{
	GENERATED_BODY()

public:
	AEnemyDrone();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void TurnToFacePlayer(float DeltaSeconds);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UDroneMovementComponent* DroneMoveComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* DeathTrailVFX;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* CrashSFX;

protected:
	virtual void OnKnock() override;
	virtual void OnDie(AActor* DeadActor) override;

	UFUNCTION()
	void OnDroneHitGround(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	                      FVector NormalImpulse, const FHitResult& Hit);

	/* ============ Hover System ============ */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Hover")
	float HoverHeight = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Hover")
	float HoverInterpSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Hover")
	float HoverTraceDistance = 2000.f;


	/* ============ Movement / Patrol ============ */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Movement")
	float FlySpeed = 500.f;

	// 좌/우 회피
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Movement")
	float DodgeSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Movement")
	float DodgeTime = 0.35f;

	FTimerHandle TimerHandle_Dodge;
	bool bDodging = false;
	FVector DodgeDirection;

	void StartDodge();
	void EndDodge();


	/* ============ Facing Player ============ */

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Drone|Movement")
	float RotationInterpSpeed = 6.f;


	/* ============ Target Info ============ */

	AActor* GetPlayerPawn() const;

	
	UPROPERTY(EditAnywhere, Category="FX")
	UNiagaraSystem* KnockSparkVFX;

	UPROPERTY(EditAnywhere, Category="Knockback")
	float KnockbackStrength = 800.f;

	FTimerHandle TimerHandle_KnockStun;

};
