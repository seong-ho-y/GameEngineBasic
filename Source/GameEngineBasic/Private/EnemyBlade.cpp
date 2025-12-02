// EnemyBlade.cpp

#include "EnemyBlade.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

AEnemyBlade::AEnemyBlade()
{
	PrimaryActorTick.bCanEverTick = true;

	// =========================
	// Root
	// =========================
	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// =========================
	// Collision (Hitbox)
	// =========================
	BladeCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BladeCollision"));
	BladeCollision->SetupAttachment(RootScene);

	// 기본 박스 크기 & 회전
	BladeCollision->InitBoxExtent(BoxExtent);
	BladeCollision->SetRelativeRotation(BoxRotation);

	// 충돌 설정
	BladeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BladeCollision->SetCollisionObjectType(ECC_GameTraceChannel5);       // EnemyBlade
	BladeCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	BladeCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Overlap); // Player
	BladeCollision->SetGenerateOverlapEvents(true);

	// Overlap 바인딩
	BladeCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBlade::OnBladeOverlap);

	// =========================
	// Mesh
	// =========================
	BladeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	BladeMesh->SetupAttachment(RootScene);
	BladeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BladeMesh->SetGenerateOverlapEvents(false);

	// =========================
	// Trail VFX
	// =========================
	TrailVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrailVFX"));
	TrailVFX->SetupAttachment(RootScene);
	TrailVFX->bAutoActivate = true;
	
}

void AEnemyBlade::BeginPlay()
{
	Super::BeginPlay();

	// BP에서 BoxExtent / BoxRotation 바꿨을 수도 있으니 한 번 더 반영
	if (BladeCollision)
	{
		BladeCollision->SetBoxExtent(BoxExtent);
		BladeCollision->SetRelativeRotation(BoxRotation);
	}
	
}

void AEnemyBlade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugHitbox && BladeCollision)
	{
		const FTransform& T = BladeCollision->GetComponentTransform();
		DrawDebugBox(
			GetWorld(),
			T.GetLocation(),
			BladeCollision->GetUnscaledBoxExtent(),
			T.GetRotation(),
			FColor::Red,
			false,
			0.f,
			0,
			1.0f
		);
	}
}

void AEnemyBlade::ActivateHitbox()
{
	AlreadyHitActors.Empty();

	if (BladeCollision)
	{
		BladeCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		BladeCollision->SetGenerateOverlapEvents(true);
	}

	StartTrail();

}

void AEnemyBlade::DeactivateHitbox()
{
	if (BladeCollision)
	{
		BladeCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		BladeCollision->SetGenerateOverlapEvents(false);
	}

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
	if (!OtherActor || OtherActor == OwnerCharacter)
	{
		return;
	}

	// 중복 히트 방지
	if (AlreadyHitActors.Contains(OtherActor))
	{
		return;
	}
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

	// 데미지 적용
	AController* InstigatorController = nullptr;
	if (OwnerCharacter)
	{
		InstigatorController = OwnerCharacter->GetController();
	}

	UGameplayStatics::ApplyDamage(
		OtherActor,
		Damage,
		InstigatorController,
		OwnerCharacter ? Cast<AActor>(OwnerCharacter) : this,
		nullptr
	);
}
