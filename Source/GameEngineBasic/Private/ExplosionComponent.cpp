#include "ExplosionComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraShakeBase.h"
#include "Engine/OverlapResult.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

UExplosionComponent::UExplosionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UExplosionComponent::Explode()
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	FVector Location = Owner->GetActorLocation();

	/* =========================
	   1. 폭발 데미지
	========================= */
	UGameplayStatics::ApplyRadialDamage(
		Owner,
		Damage,
		Location,
		Radius,
		DamageType,
		TArray<AActor*>(), // damage ignore list
		Owner,
		Owner->GetInstigatorController(),
		true
	);

	/* =========================
	   2. 폭발 이펙트
	========================= */
	if (ExplosionFX)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ExplosionFX,
			Location
		);
	}

	/* =========================
	   3. 폭발 사운드
	========================= */
	if (ExplosionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			ExplosionSound,
			Location
		);
	}

	/* =========================
	   4. 카메라 흔들림
	========================= */
	if (CameraShake)
	{
		UGameplayStatics::PlayWorldCameraShake(
			GetWorld(),
			CameraShake,
			Location,
			0.f,
			Radius
		);
	}

	/* =========================
	   5. 물리 오브젝트 밀기
	========================= */
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Location,
		FQuat::Identity,
		ECC_PhysicsBody,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	for (auto& Hit : Overlaps)
	{
		if (UPrimitiveComponent* Comp = Hit.GetComponent())
		{
			if (Comp->IsSimulatingPhysics())
			{
				Comp->AddRadialImpulse(
					Location,
					Radius,
					ImpulseStrength,
					ERadialImpulseFalloff::RIF_Linear,
					true
				);
			}
		}
	}

	/* =========================
	   6. 자폭 드론 제거
	========================= */
	Owner->Destroy();
}
