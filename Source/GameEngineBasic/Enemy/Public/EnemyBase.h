// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GameEngineBasic/Components/public/Attack.h"
#include "GameEngineBasic/Components/public/Damageable.h"
#include "GameFramework/Pawn.h"
#include "EnemyBase.generated.h"

class UFloatingPawnMovement;
class USphereComponent;
class UStaticMeshComponent;
class UHealthComp;
class UShooterComp;
class AAIController;
class UBlackboardComponent;


UCLASS()
class GAMEENGINEBASIC_API AEnemyBase : public APawn, public IDamageable, public IAttack
{
	GENERATED_BODY()

	
public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	UStaticMeshComponent* MeshComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UFloatingPawnMovement* MovementComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	UHealthComp* HealthComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	UShooterComp* ShooterComp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float MaxSpeed = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float BaseAcceleration = 2.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
	float Deceleration = 1.2f;
	float MaxHealth;
	float CurrentHealth;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	void PossessedBy(AController* NewController);

	
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void ApplyDamage_Implementation(float DamageAmount, AController* InstigatorController, AActor* DamageCauser, FVector HitLoc, TSubclassOf<UDamageType> DamageType) override;
	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                 AActor* DamageCauser);

	virtual void Die_Implementation(AActor* Killer) override;

	virtual bool IsDead_Implementation() const override;

	virtual void Attack_Implementation() override;
	void Activate();
	void DeActivate();
	void SpawnVfx(UNiagaraSystem* Vfx);

protected:
	// AI 컨트롤러와 블랙보드에 대한 참조를 저장해둘 변수 (매번 찾는 것보다 효율적)
	UPROPERTY()
	TObjectPtr<AAIController> AICache;

	UPROPERTY()
	TObjectPtr<UBlackboardComponent> BlackboardCache;

	// 디자이너가 블루프린트에서 쉽게 수정할 수 있도록 이동 관련 수치를 노출
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float ThrustStrength = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float StrafeStrength = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement Stats")
	float BoostMultiplier = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vfx")
	UNiagaraSystem* DamagedVfx;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vfx")
	UNiagaraSystem* ExplosionVfx;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vfx")
	UNiagaraSystem* AttackVfx;
};
