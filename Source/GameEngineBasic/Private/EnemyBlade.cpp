#include "EnemyBlade.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyBlade::AEnemyBlade()
{
	PrimaryActorTick.bCanEverTick = false;

	// Root
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = RootScene;

	// Collision
	BladeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BladeCollision"));
	BladeCollision->SetupAttachment(RootComponent);
	// 공격 박스 사이즈
	BladeCollision->InitBoxExtent(FVector(60.f, 20.f, 20.f));   // 앞/좌/위

	// X 방향으로 박스 회전 -> 칼 끝 방향으로 판정이 생김
	BladeCollision->SetRelativeRotation(FRotator(0.f, 90.f, 0.f));

	// Collision 설정
	BladeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BladeCollision->SetCollisionObjectType(ECC_GameTraceChannel2);
	BladeCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	BladeCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap);

	// Overlap 바인딩
	BladeCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBlade::OnBladeOverlap);

	// Mesh
	BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	BladeMesh->SetupAttachment(RootComponent);
	BladeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BladeMesh->bEditableWhenInherited = true;

	// VFX
	TrailVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailVFX"));
	TrailVFX->SetupAttachment(RootComponent);
}


void AEnemyBlade::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEnemyBlade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEnemyBlade::ActivateHitbox()
{
	AlreadyHitActors.Empty();
	BladeCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	StartTrail();
}

void AEnemyBlade::DeactivateHitbox()
{
	BladeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	StopTrail();
}

void AEnemyBlade::StartTrail()
{
	if (TrailVFX && !TrailVFX->IsActive())
	{
		TrailVFX->Activate();
	}
}

void AEnemyBlade::StopTrail()
{
	if (TrailVFX && TrailVFX->IsActive())
	{
		TrailVFX->Deactivate();
	}
}

void AEnemyBlade::ResetHitList()
{
	AlreadyHitActors.Empty();
}

void AEnemyBlade::OnBladeOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == OwnerCharacter) return;

	// 중복 히트 방지
	if (AlreadyHitActors.Contains(OtherActor))
		return;

	AlreadyHitActors.Add(OtherActor);

	// Hit Spark
	if (HitSparkVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			HitSparkVFX,
			SweepResult.ImpactPoint
		);
	}

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		OwnerCharacter ? OwnerCharacter->GetInstigatorController() : nullptr,
		OwnerCharacter,
		nullptr
	);
}
