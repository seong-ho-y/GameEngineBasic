// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ShieldComp.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "SpaceCharacter/Shield/ShieldActor.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"

UShieldComp::UShieldComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UShieldComp::BeginPlay()
{
	Super::BeginPlay();

	HealthComp = GetOwner()->FindComponentByClass<UHealthComp>();

	if (HealthComp)
	{
		HealthComp->OnShieldBroken.AddDynamic(this, &UShieldComp::OnShieldBrokenHandler);
	}

	if (ShieldActorClass)
	{
		AActor* Owner = GetOwner();
		FActorSpawnParameters Params;
		Params.Owner = Owner;

		ShieldActor = Owner->GetWorld()->SpawnActor<AShieldActor>(
			ShieldActorClass,
			Owner->GetActorLocation(),
			Owner->GetActorRotation(),
			Params
		);

		ShieldActor->AttachToComponent(
			Cast<ACharacter>(Owner)->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			FName("Shield")
		);

		ShieldActor->SetActorHiddenInGame(true);
		ShieldActor->SetActorEnableCollision(false);
		ShieldActor->OwnerCharacter = Owner;
	}
}


void UShieldComp::ActivateShield()
{
	if (!HealthComp) return;
	if (bShieldActive) return;

	OnShieldActivated.Broadcast();
	bShieldActive = true;

	HealthComp->bUseShield = true;
	HealthComp->bUseShieldRegen = false;
	HealthComp->CurrentShield = FMath::Clamp(ShieldAmount, 0, HealthComp->MaxShield);
	HealthComp->ForceBroadcast();
}

void UShieldComp::DeactivateShield()
{
	if (!HealthComp) return;

	OnShieldDeactivated.Broadcast();
	bShieldActive = false;
	HealthComp->CurrentShield = 0;
	HealthComp->bUseShield = false;
	HealthComp->bUseShieldRegen = false;

	HealthComp->ForceBroadcast();
}

void UShieldComp::OnShieldBrokenHandler(AActor* Owner)
{
	if (bShieldActive)
	{
		DeactivateShield();
	}
}

