// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/PartUnlockItem.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"

#include "Blueprint/UserWidget.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "MyPlayerState.h"
#include "GameEngineBasic/System/SaveSystemManager.h"

#include "Curves/CurveFloat.h"
#include "Kismet/GameplayStatics.h"

APartUnlockItem::APartUnlockItem()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAll"));
	CollisionSphere->InitSphereRadius(300.f);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionSphere->SetupAttachment(SceneRoot);

	BoxBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxBody"));
	BoxBody->SetupAttachment(SceneRoot);

	BoxLid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxLid"));
	BoxLid->SetupAttachment(BoxBody);

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
	InteractWidget->SetupAttachment(SceneRoot);
	InteractWidget->SetWidgetSpace(EWidgetSpace::World);
	InteractWidget->SetDrawSize(FVector2D(128.f, 128.f));
	InteractWidget->SetVisibility(false);

	Effect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
	Effect->SetupAttachment(SceneRoot);
	Effect->bAutoActivate = true;
}

void APartUnlockItem::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &APartUnlockItem::OnOverlapEnd);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APartUnlockItem::OnOverlapBegin);

	if (LidOpenCurve)
	{
		FOnTimelineFloat Progress;
		Progress.BindUFunction(this, FName("HandleLidOpenProgress"));

		LidOpenTimeline.AddInterpFloat(LidOpenCurve, Progress);
	}
}

void APartUnlockItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	LidOpenTimeline.TickTimeline(DeltaTime);
}

void APartUnlockItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		Character->CurrentInteractTarget = this;

		if (InteractWidget && !bActivated)
			InteractWidget->SetVisibility(true);
	}
}

void APartUnlockItem::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		if (Character->CurrentInteractTarget == this)
			Character->CurrentInteractTarget = nullptr;

		if (InteractWidget)
			InteractWidget->SetVisibility(false);
	}
}

void APartUnlockItem::HandleLidOpenProgress(float Value)
{
	FRotator NewRotation = FRotator(Value, 0.f, 0.f);
	BoxLid->SetRelativeRotation(NewRotation);
}

void APartUnlockItem::Interact(ASpaceCharacter* Character)
{
	if (!Character || PartRowName.IsNone())
		return;

	AMyPlayerState* PS = Character->GetPlayerState<AMyPlayerState>();
	if (!PS)
		return;

	if (InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 1) 파츠 해금
	PS->UnlockPart(PartRowName);

	// 2) 인벤토리 장착
	if (PS->Inventory)
	{
		PS->Inventory->EquipPart(Slot, PartRowName);
	}
	if (APawn* Pawn = Cast<APawn>(Character))
	{
		USaveSystemManager::SavePawnState(Pawn);
	}
	LidOpenTimeline.Play();

	if (AbilityUI)
	{
		if (UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), AbilityUI))
		{
			Widget->AddToViewport(100); // ZOrder 높게

			TWeakObjectPtr<UUserWidget> WeakWidget(Widget);

			GetWorld()->GetTimerManager().SetTimer(
				RemoveTimer,
				FTimerDelegate::CreateLambda([WeakWidget]()
					{
						if (UUserWidget* StrongWidget = WeakWidget.Get())
						{
							StrongWidget->RemoveFromParent();
						}
					}),
				3.0f,
				false
			);
		}
	}


	if (Effect)
		Effect->Deactivate();

}