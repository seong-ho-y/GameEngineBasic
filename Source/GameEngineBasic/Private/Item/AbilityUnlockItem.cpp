// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AbilityUnlockItem.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "SpaceCharacter/SpaceCharacter.h"

// Sets default values
AAbilityUnlockItem::AAbilityUnlockItem()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionSphere->InitSphereRadius(80.f);
	RootComponent = CollisionSphere;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(RootComponent);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AAbilityUnlockItem::OnOverlapBegin
	);

}

void AAbilityUnlockItem::BeginPlay()
{
	Super::BeginPlay();	
}

void AAbilityUnlockItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		Character->UnlockAbility(AbilityToUnlock);

		Destroy();
	}
}


