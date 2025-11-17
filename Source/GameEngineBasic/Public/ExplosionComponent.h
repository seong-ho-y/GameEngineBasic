#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ExplosionComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEENGINEBASIC_API UExplosionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UExplosionComponent();

	/* ===== 폭발 파라미터 ===== */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	float Damage = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	float Radius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	float ImpulseStrength = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	TSubclassOf<UDamageType> DamageType;

	/* ===== VFX / SFX ===== */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	USoundBase* ExplosionSound;

	/* ===== 카메라 흔들림 ===== */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Explosion")
	TSubclassOf<UCameraShakeBase> CameraShake;

public:
	/* 메인 기능 */
	UFUNCTION(BlueprintCallable)
	void Explode();
};
