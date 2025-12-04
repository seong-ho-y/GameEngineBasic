

#include "Portal.h"
#include "GameMode/MyGameMode.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"

#include "GameEngineBasic/System/SaveSystemManager.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
    
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->InitSphereRadius(120.f);
    CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    CollisionSphere->SetupAttachment(SceneRoot);

	Cylinder = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cylinder"));
	Cylinder->SetupAttachment(SceneRoot);

    Effect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Effect"));
    Effect->SetupAttachment(SceneRoot);
    Effect->bAutoActivate = true;
}

void APortal::BeginPlay()
{
	Super::BeginPlay();
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnPortalOverlap);
}

void APortal::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APortal::OnPortalOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if(auto Char = Cast<APawn>(OtherActor))
    {
        USaveSystemManager::SavePawnState(Char);
	}
    if (AMyGameMode* GM = Cast<AMyGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        GM->RequestStageTransition(DestinationLevelName);
    }
}