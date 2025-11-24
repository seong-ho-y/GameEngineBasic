// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "Camera/CameraComponent.h"

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

	if (WeaponTable && WeaponRowName != NAME_None)
	{
		const FWeaponData* Row = WeaponTable->FindRow<FWeaponData>(WeaponRowName, TEXT(""));
		if (Row)
		{
			WeaponData = *Row;
			
		}
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Blue,
	FString::Printf(TEXT("WeaponTable=%s | RowName=%s | hasRow=%d"),
		*WeaponTable->GetName(),
		*WeaponRowName.ToString(),
		Row != nullptr));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Red,TEXT("NoWeapon"));
	}
	
	// 1) ShooterComp에 무기 스탯 적용
	ShooterComp->PendingDamage = WeaponData.Damage;
	ShooterComp->ReloadTime = WeaponData.ReloadTime;
	ShooterComp->CurrentAmmo =  WeaponData.FullAmmo;
	ShooterComp->FullAmmo = WeaponData.FullAmmo;
	ShooterComp->MaxAmmo = WeaponData.MaxAmmo;
	ShooterComp->ProjectileClass = WeaponData.ProjectileClass;

	SpawnAndAttachWeaponMesh();
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

void UWeaponComponent::SetWeaponMesh(UStaticMeshComponent* InWeaponMeshComp)
{
	WeaponMeshComp = InWeaponMeshComp;
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
	if (!ShooterComp || !OwnerCharacter)
		return;
	
	ShooterComp->SetFireDirection(GetAimDirection());
	ShooterComp->TryFire();

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
	// 1) 무기 메쉬가 있으면 무기 메쉬 소켓 사용
	if (WeaponMeshComp)
	{
		return WeaponMeshComp->GetSocketLocation(MuzzleSocketName);
	}

	// 2) 없으면 플레이어 메쉬 fallback
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		return OwnerCharacter->GetMesh()->GetSocketLocation(MuzzleSocketName);
	}

	return FVector::ZeroVector;
}

void UWeaponComponent::SpawnAndAttachWeaponMesh()
{
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
		return;

	if (!WeaponData.WeaponMesh)   // DataTable에 SkeletalMesh 또는 StaticMesh 넣어두기
		return;

	if (!WeaponMeshComp)
	{
		WeaponMeshComp = NewObject<UStaticMeshComponent>(OwnerCharacter);
		WeaponMeshComp->RegisterComponent();
	}

	WeaponMeshComp->SetStaticMesh(WeaponData.WeaponMesh);

	// 무기 소켓이름은 DT 또는 Blueprint로 지정
	FName SocketName = TEXT("WeaponSocket");

	WeaponMeshComp->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		SocketName
	);
}