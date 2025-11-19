// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::InitializeWeapon(ASpaceCharacter* Player, UShooterComp* InShooterComp)
{
	OwnerCharacter = Player;
	ShooterComp = InShooterComp;
}


FVector UWeaponComponent::GetAimDirection() const
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponComp : No Player Found!"));
		return FVector::ForwardVector;
	}
	if (!ShooterComp) return FVector::ForwardVector;
	const FVector AimPoint = GetAimPoint();
	const FVector MuzzleLoc = GetMuzzleLoc();
	ShooterComp->SetMuzzle(MuzzleLoc);
	return (AimPoint - MuzzleLoc).GetSafeNormal();
}

void UWeaponComponent::HandleFirePressed()
{
	PerformFire();
}

void UWeaponComponent::HandleFireReleased()
{
	
}

bool UWeaponComponent::CanFire() const
{
	return true;
}

void UWeaponComponent::PerformFire()
{
	ShooterComp->SetFireDirection(GetAimDirection());
	
}

FVector UWeaponComponent::GetAimPoint() const
{
	if (!OwnerCharacter) return FVector::ZeroVector;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return FVector::ZeroVector;

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);

	FVector CamOrigin;
	FVector CamDirection;

	if (!PC->DeprojectScreenPositionToWorld(
		SizeX * 0.5f,
		SizeY * 0.5f,
		CamOrigin,
		CamDirection))
	{
		return FVector::ZeroVector;
	}
	const float TraceDistance = 50000.f;
	FVector TraceEnd = CamOrigin + CamDirection * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter); 

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		CamOrigin,
		TraceEnd,
		ECC_Visibility,
		Params);
	
	FVector AimPoint = bHit ? Hit.Location : TraceEnd;

	// TPS 근거리 보정 (필수!)
	float Dist = FVector::Dist(CamOrigin, AimPoint);
	if (Dist < 150.f)  // 벽 1.5m 안쪽일 때
	{
		AimPoint = CamOrigin + CamDirection * 2000.f;
	}

	return AimPoint;
}

FVector UWeaponComponent::GetMuzzleLoc() const
{
	FVector SpawnLoc;
	
	// 1. 스켈레탈 메쉬를 먼저 확인
	USceneComponent* MuzzleComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();

	// 2. 스켈레탈 메쉬 없으면 스태틱 메쉬 확인
	if (!MuzzleComp)
		MuzzleComp = GetOwner()->FindComponentByClass<UStaticMeshComponent>();

	// 3. Mesh에서 MuzzleSocketName에 맞는 Muzzle 위치 가져오기
	if (MuzzleComp)
	{
		SpawnLoc = MuzzleComp->GetSocketLocation(MuzzleSocketName);
	}
	return SpawnLoc;
}
