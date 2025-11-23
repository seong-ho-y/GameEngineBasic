// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AbilityUnlockItem.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"

#include "Blueprint/UserWidget.h"
#include "SpaceCharacter/SpaceCharacter.h"

#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAbilityUnlockItem::AAbilityUnlockItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionSphere->InitSphereRadius(300.f);
	CollisionSphere->SetupAttachment(SceneRoot);

	BoxBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxBody"));
	BoxBody->SetupAttachment(SceneRoot);

	BoxLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxLid"));
	BoxLid->SetupAttachment(BoxBody);

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(
		this,
		&AAbilityUnlockItem::OnOverlapBegin
	);

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(SceneRoot);
	InteractWidget->SetWidgetSpace(EWidgetSpace::World);
	InteractWidget->SetDrawSize(FVector2D(128.f, 128.f));
	InteractWidget->SetVisibility(false);

	Effect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
	Effect->SetupAttachment(SceneRoot);
	Effect->bAutoActivate = true;
}

void AAbilityUnlockItem::BeginPlay()
{
	Super::BeginPlay();	

	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AAbilityUnlockItem::OnOverlapEnd);

	if (LidOpenCurve)
	{
		FOnTimelineFloat Progress;
		Progress.BindUFunction(this, FName("HandleLidOpenProgress"));

		LidOpenTimeline.AddInterpFloat(LidOpenCurve, Progress);
	}
}

void AAbilityUnlockItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	LidOpenTimeline.TickTimeline(DeltaTime);
}

void AAbilityUnlockItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		Character->CurrentInteractTarget = this;

		if (InteractWidget)
			InteractWidget->SetVisibility(true);
	}
}

void AAbilityUnlockItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		if (Character->CurrentInteractTarget == this)
			Character->CurrentInteractTarget = nullptr;

		if (InteractWidget)
			InteractWidget->SetVisibility(false);
	}
}

void AAbilityUnlockItem::Interact(ASpaceCharacter* Character)
{
	Character->UnlockAbility(AbilityToUnlock);

	if (InteractWidget)
		InteractWidget->SetVisibility(false);

	LidOpenTimeline.Play();

	if (Effect)
		Effect->Deactivate();
}

void AAbilityUnlockItem::HandleLidOpenProgress(float Value)
{
	float OpenAngle = FMath::Lerp(30.f, 0.f, Value);
	BoxLid->SetRelativeRotation(FRotator(0.f, 0.f, OpenAngle));
}


