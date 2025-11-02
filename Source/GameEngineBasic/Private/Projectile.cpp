// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "GameEngineBasic/Components/public/Damageable.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AProjectile::AProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	CollisionComp->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);

	// Set as root component
	RootComponent = CollisionComp;

	// StaticMesh
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	
	InitialLifeSpan = 10.0f;
	

	// Vfx
	Vfx = CreateDefaultSubobject<UNiagaraComponent>(TEXT("VFXComp"));
	Vfx->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (TrailEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(
			TrailEffect,
			RootComponent,
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 나 자신이나 나를 쏜 소유자(Owner)와 부딪힌 경우는 무시
	AActor* MyOwner = GetOwner();
	if (OtherActor == nullptr || OtherActor == this || OtherActor == MyOwner)
	{
		return;
	}
	// 1. 부딪힌 대상이 IDamageable인지 확인
	if (OtherActor->GetClass()->ImplementsInterface(UDamageable::StaticClass()))
	{
		AController* InstigatorController = MyOwner ? MyOwner->GetInstigatorController() : nullptr;
		// 나중에 데미지는 무기 컴포넌트에서 가져와서 DamageAmount에 넣어줄거임
		constexpr float DamageAmount = 5.0f; // 이 프로젝타일의 기본 데미지
		

		UGameplayStatics::ApplyDamage(
			OtherActor,
			DamageAmount,
			InstigatorController,
			this,
			nullptr);
	}
	
	if (ImpactEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactEffect,
			GetActorLocation(),
			GetActorRotation(),
			true
		);
	}
	// 2. 물리 시뮬레이션 중인 다른 오브젝트에 대한 기존 로직도 유지
	if (Hit.GetComponent() != nullptr && Hit.GetComponent()->IsSimulatingPhysics())
	{
		Hit.GetComponent()->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
	}
	
	// 충돌 시 파괴
	Destroy();
}

