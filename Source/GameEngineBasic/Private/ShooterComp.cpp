// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterComp.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UShooterComp::UShooterComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UShooterComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UShooterComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UShooterComp::TryFire()
{
	if (CanFire())
	{
		Fire();
		return true;
	}
	// 발사 실패 사운드
	return false;
}

bool UShooterComp::CanFire_Implementation() const
{
	// 1. Check Ammo
	if (CurrentAmmo <= 0)
	{
		return false;
	}
	// 2. Check Cooldown
	if (!bIsReadyToFire)
	{
		return false;
	}
	
	return true;
}

void UShooterComp::Fire_Implementation()
{
	// 0. Check IsValid
	AActor* MyOwner = GetOwner();
	if (!MyOwner || !ProjectileClass)
	{
		return;
	}
	// 1. Use Ammo
	CurrentAmmo--;
	// 2. Fire Cooldown Start
	bIsReadyToFire = false;
	GetWorld()->GetTimerManager().SetTimer(
		FireRateTimerHandle,
		this,
		&UShooterComp::ResetFireReady,
		FireRate,
		false);
	// 3. Calculate Spawn Loc & Rot
	USceneComponent* MuzzleSocket = MyOwner->FindComponentByClass<UStaticMeshComponent>();
	const FVector SpawnLocation = MuzzleSocket ? MuzzleSocket->GetSocketLocation(MuzzleSocketName): MyOwner->GetActorLocation();
	const FRotator SpawnRotation = MuzzleSocket ? MuzzleSocket->GetSocketRotation(MuzzleSocketName): MyOwner->GetActorRotation();

	// 4. Projectile Spawn
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = MyOwner; // Set MyOwner to Projectile's Owner
	SpawnParams.Instigator = MyOwner->GetInstigator(); // Set MyOwner's Instigator to Projectile's Instigator

	GetWorld()->SpawnActor<AProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);
	
	//Play VFX and Sound
	if (MuzzleFlashEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashEffect, SpawnLocation, SpawnRotation);
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), FireSound, SpawnLocation);
	}
}

void UShooterComp::ResetFireReady()
{
	bIsReadyToFire = true;
}
