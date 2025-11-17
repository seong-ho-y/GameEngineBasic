#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Components/SphereComponent.h>
#include "ShieldActor.generated.h"

UCLASS()
class GAMEENGINEBASIC_API AShieldActor : public AActor
{
	GENERATED_BODY()
	
public:
    AShieldActor();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(VisibleAnywhere)
    USphereComponent* ShieldCollision;

    UPROPERTY(EditAnywhere)
    UParticleSystem* HitEffect;

    AActor* OwnerCharacter;

    void ActivateShield();
    void DeactivateShield();

private:
    UFUNCTION()
    void OnShieldHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);
};
