// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Projectile.generated.h"

class USphereComponent;

UCLASS(config = Game)
class GAMEENGINEBASIC_API AProjectile : public AActor
{
	GENERATED_BODY()


public:
	// Sets default values for this actor's properties
	AProjectile();

	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
	USphereComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Projectile)
	UStaticMeshComponent* MeshComp;


	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DamageAmount = 5.0f;

	// VFX
	UPROPERTY(EditDefaultsOnly, Category = VFX)
	UNiagaraComponent* Vfx;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Cascade")
	UParticleSystem* TrailEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Cascade")
	UParticleSystem* ImpactEffect;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Projectile")
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};
