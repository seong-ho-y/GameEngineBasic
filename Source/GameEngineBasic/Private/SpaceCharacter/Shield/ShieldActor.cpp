// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/Shield/ShieldActor.h"
#include "GameEngineBasic/Public/Projectile.h"

AShieldActor::AShieldActor()
{
    PrimaryActorTick.bCanEverTick = false;

    ShieldCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldCollision"));
    RootComponent = ShieldCollision;

    ShieldCollision->InitSphereRadius(120.f);
    ShieldCollision->SetCollisionProfileName(TEXT("ShieldActor")); 
    ShieldCollision->SetGenerateOverlapEvents(false);
    ShieldCollision->SetNotifyRigidBodyCollision(true);
    ShieldCollision->OnComponentHit.AddDynamic(this, &AShieldActor::OnShieldHit);
}

void AShieldActor::BeginPlay()
{
}

void AShieldActor::OnShieldHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (!OtherActor) return;

    AProjectile* Projectile = Cast<AProjectile>(OtherActor);
    if (Projectile)
    {
        if (HitEffect)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                HitEffect,
                Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation()
            );
        }

        Projectile->Destroy();
    }
}
