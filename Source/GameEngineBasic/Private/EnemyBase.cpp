// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/Public/EnemyBase.h"

// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyBase::ApplyDamage_Implementation(float DamageAmount, AController* InstigatorController, AActor* DamageCauser,
	FVector HitLoc, TSubclassOf<UDamageType> DamageType)
{
	IDamageable::ApplyDamage_Implementation(DamageAmount, InstigatorController, DamageCauser, HitLoc, DamageType);
}

void AEnemyBase::Die_Implementation(AActor* Killer)
{
	IDamageable::Die_Implementation(Killer);
}

bool AEnemyBase::IsDead_Implementation() const
{
	return IDamageable::IsDead_Implementation();
}

void AEnemyBase::Attack_Implementation()
{
	IAttack::Attack_Implementation();
}

