#include "EnemyBombDrone.h"

#include "ExplosionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "SpaceCharacter/SpaceCharacter.h"

AEnemyBombDrone::AEnemyBombDrone()
{

	ExplosionComp = CreateDefaultSubobject<UExplosionComponent>(TEXT("ExplosionComp"));
	// CharacterMovement 제거
	if (GetCharacterMovement())
		GetCharacterMovement()->DestroyComponent();
	DroneMoveComp->bHovering = false;
}

void AEnemyBombDrone::BeginPlay()
{
	Super::BeginPlay();

	Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}
void AEnemyBombDrone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Player || !ExplosionComp) return;

	float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());

	if (Dist<=ExplodeDistance)
		ExplosionComp->Explode();
}
