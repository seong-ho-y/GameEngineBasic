// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Bonfire.h"

#include "GameEngineBasic/System/SaveSystemManager.h"
#include "SpaceCharacter/SpaceCharacter.h"

#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"


#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABonfire::ABonfire()
{
	PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // ----- Collision Sphere -----
    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(300.f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
   // CollisionSphere->SetCollisionResponseToChannel(ECC_Player, ECR_Overlap);
	CollisionSphere->SetupAttachment(RootComponent);

    // ----- Mesh -----
    BonfireMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BonfireMesh"));
    BonfireMesh->SetupAttachment(RootComponent);

    InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractWidget"));
    InteractWidget->SetupAttachment(RootComponent);
    InteractWidget->SetWidgetSpace(EWidgetSpace::World);
    InteractWidget->SetDrawSize(FVector2D(128.f, 128.f));
    InteractWidget->SetVisibility(false);

    Effect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
    Effect->SetupAttachment(SceneRoot);
    Effect->bAutoActivate = true;
}

// Called when the game starts or when spawned
void ABonfire::BeginPlay()
{
	Super::BeginPlay();
	
    CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ABonfire::OnOverlapEnd);
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABonfire::OnOverlapBegin);
}

// Called every frame
void ABonfire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABonfire::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
    {
        Character->CurrentInteractTarget = this;

        if (InteractWidget && !bActivated)
            InteractWidget->SetVisibility(true);
    }
}

void ABonfire::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex)
{
    if (ASpaceCharacter* Character = Cast<ASpaceCharacter>(OtherActor))
    {
        if (Character->CurrentInteractTarget == this)
        {
            Character->CurrentInteractTarget = nullptr;
            if (InteractWidget)
                InteractWidget->SetVisibility(false);
        }
    }
}

void ABonfire::StartNiagara(ASpaceCharacter* Character)
{
    if (Character->HealVfx)
    {
        Character->HealVfx->SetActive(true, true);
        Character->HealVfx->Activate(true);
    }

    GetWorldTimerManager().SetTimer(
        NiagaraTimerHandle,
        this,
        &ABonfire::StopNiagara,
        0.2,
        false
    );
}

void ABonfire::StopNiagara()
{
	auto Char = Cast<ASpaceCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (Char)
    {
        Char->HealVfx->Deactivate();
	}
}

void ABonfire::Interact(ASpaceCharacter* Character)
{
    if (!Character) return;

    Character->GetHealthComponent()->RestoreFullHealth();
    StartNiagara(Character);
    bActivated = true;
    
    // Save State And Location)
    USaveSystemManager::SavePawnState(Character);

	// UI   
    if (InteractWidget)
        InteractWidget->SetVisibility(false);

    if (AbilityUI)
    {
        if (UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), AbilityUI))
        {
            Widget->AddToViewport(100); // ZOrder 높게

            // 3초 뒤 자동 제거
            FTimerHandle RemoveTimer;
            GetWorld()->GetTimerManager().SetTimer(
                RemoveTimer,
                FTimerDelegate::CreateLambda([Widget]()
                    {
                        Widget->RemoveFromParent();
                    }),
                3.0f,
                false
            );
        }
    }

	// FX
    if (Effect)
        Effect->Deactivate();
}