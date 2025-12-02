// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Bonfire.h"
#include "GameEngineBasic/System/SaveSystemManager.h"
#include "SpaceCharacter/SpaceCharacter.h"

#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "NiagaraComponent.h"

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

        if (InteractWidget)
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

void ABonfire::Interact(ASpaceCharacter* Character)
{
    if (!Character) return;

    Character->GetHealthComponent()->RestoreFullHealth();
    // ���� Ȱ��ȭ
    bActivated = true;

    // 1) ���� ��ġ�� SpawnPoint�� ����
    USaveSystemManager::SaveSpawnPoint(GetActorLocation(), GetActorRotation());

    // 2) ĳ���� ������ ����
    USaveSystemManager::SavePawnState(Character);

	// 3) UI �� ����Ʈ ó��   
    if (InteractWidget)
        InteractWidget->SetVisibility(false);

    if (Effect)
        Effect->Deactivate();
}