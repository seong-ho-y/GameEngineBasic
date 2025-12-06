// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AbilityUnlockItem.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"

#include "Blueprint/UserWidget.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameEngineBasic/System/SaveSystemManager.h"

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

void AAbilityUnlockItem::BeginPlay()
{
	Super::BeginPlay();	

	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AAbilityUnlockItem::OnOverlapEnd);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAbilityUnlockItem::OnOverlapBegin);
	
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

		if (InteractWidget && !bActivated)
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

	bActivated = true;

	if (InteractWidget)
	{
		InteractWidget->SetVisibility(false);
	}

	if (APawn* Pawn = Cast<APawn>(Character)) // APawn으로 캐스팅
	{
		USaveSystemManager::SavePawnState(Pawn);  // APawn을 인수로 전달
		UE_LOG(LogTemp, Log, TEXT("Weapon unlocked and state saved."));
	}

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

	LidOpenTimeline.Play();

	if (Effect)
		Effect->Deactivate();
}

void AAbilityUnlockItem::HandleLidOpenProgress(float Value)
{
	float OpenAngle = FMath::Lerp(30.f, 0.f, Value);
	BoxLid->SetRelativeRotation(FRotator(0.f, 0.f, OpenAngle));
}


