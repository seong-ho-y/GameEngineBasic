// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Attack.h"
#include "Damageable.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

UCLASS()
class GAMEENGINEBASIC_API AEnemyBase : public ACharacter, public IDamageable, public IAttack
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void ApplyDamage_Implementation(float DamageAmount, AController* InstigatorController, AActor* DamageCauser, FVector HitLoc, TSubclassOf<UDamageType> DamageType) override;

	virtual void Die_Implementation(AActor* Killer) override;

	virtual bool IsDead_Implementation() const override;

	virtual void Attack_Implementation() override;
};
