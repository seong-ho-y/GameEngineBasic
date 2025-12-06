// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/WeaponItemBox.h"

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

AWeaponItemBox::AWeaponItemBox()
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

void AWeaponItemBox::BeginPlay()
{
	Super::BeginPlay();
	
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponItemBox::OnOverlapEnd);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponItemBox::OnOverlapBegin);

	if (LidOpenCurve)
	{
		FOnTimelineFloat Progress;
		Progress.BindUFunction(this, FName("HandleLidOpenProgress"));

		LidOpenTimeline.AddInterpFloat(LidOpenCurve, Progress);
	}
}

void AWeaponItemBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	LidOpenTimeline.TickTimeline(DeltaTime);
}

void AWeaponItemBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		Character->CurrentInteractTarget = this;

		if (InteractWidget && !bActivated)
			InteractWidget->SetVisibility(true);
	}
}

void AWeaponItemBox::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
	if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
	{
		if (Character->CurrentInteractTarget == this)
			Character->CurrentInteractTarget = nullptr;

		if (InteractWidget)
			InteractWidget->SetVisibility(false);
	}
}

void AWeaponItemBox::HandleLidOpenProgress(float Value)
{
	FRotator NewRotation = FRotator(Value, 0.f, 0.f);
	BoxLid->SetRelativeRotation(NewRotation);
}

void AWeaponItemBox::Interact(ASpaceCharacter* Character)
{
	if (!Character || WeaponName.IsNone())
		return;
	bActivated = true;

	if (InteractWidget)
		InteractWidget->SetVisibility(false);

	if (AMyPlayerState* PS = Cast<AMyPlayerState>(Character->GetPlayerState()))
	{
		PS->UnlockWeapon(WeaponName);

		if (APawn* Pawn = Cast<APawn>(Character)) 
		{
			USaveSystemManager::SavePawnState(Pawn);
		}
		LidOpenTimeline.PlayFromStart();

		LidOpenTimeline.Play();

		if (AbilityUI)
		{
			if (UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), AbilityUI))
			{
				Widget->AddToViewport(100); // ZOrder ³ô°Ô

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
}