// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ShieldComp.h"
#include "GameFramework/Character.h"

#include "Components/SphereComponent.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Components/ChildActorComponent.h"

#include "GameEngineBasic/Public/Projectile.h"
#include "Engine/Engine.h"

UShieldComp::UShieldComp()
{
	PrimaryComponentTick.bCanEverTick = true;

	ShieldCollision = CreateDefaultSubobject<USphereComponent>(TEXT("ShieldCollision"));
}

void UShieldComp::BeginPlay()
{
	Super::BeginPlay();
	CurrentShield = MaxShield;

	AActor* Owner = GetOwner();
	if (!Owner) return;

	if (ShieldCollision)
	{
		// 2. 컴포넌트 등록 및 설정
		ShieldCollision->RegisterComponent();
		ShieldCollision->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		// 3. 콜리전 설정
		ShieldCollision->InitSphereRadius(120.f); 
		ShieldCollision->SetCollisionProfileName(TEXT("ShieldActor")); 
		ShieldCollision->SetNotifyRigidBodyCollision(true);

		// 4. 이벤트 바인딩
		ShieldCollision->OnComponentHit.AddDynamic(this, &UShieldComp::OnShieldHit);

		// 5. 초기 상태: 꺼짐 (충돌 없음, 숨김)
		ShieldCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShieldCollision->SetHiddenInGame(true);
	}
}

void UShieldComp::ActivateShield()
{
	if (!ShieldCollision) return;
	StartShieldCoolDown();

	if (CurrentShield <= 0.f)
	{
		CurrentShield = MaxShield;
	}
	bShieldActive = true;

	ShieldCollision->SetHiddenInGame(false);
	ShieldCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	OnShieldActivated.Broadcast();
}

void UShieldComp::DeactivateShield()
{
	/*
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Shield Deactivated"));
	*/
	if (!ShieldCollision) return;

	bShieldActive = false;

	ShieldCollision->SetHiddenInGame(true);
	ShieldCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	OnShieldDeactivated.Broadcast();

}

void UShieldComp::OnShieldHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 발사체 확인
	AProjectile* Projectile = Cast<AProjectile>(OtherActor);
	if (!Projectile) return;

	// 2. 데미지 전달
	// Owner의 TakeDamage -> HealthComp -> ShieldComp::ApplyShieldDamage 
	// 순서로 호출되어 로직 흐름이 유지됩니다.
	if (AActor* Owner = GetOwner())
	{
		float Damage = Projectile->DamageAmount;
		UGameplayStatics::ApplyDamage(Owner, Damage, nullptr, Owner, UDamageType::StaticClass());
	}

	// 3. 피격 이펙트 재생
	if (bShieldActive)
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
	}
	else 
	{
		if (ShieldBroken)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ShieldBroken,
				Hit.ImpactPoint,
				Hit.ImpactNormal.Rotation()
			);
		}
	}
		
	Projectile->Destroy();
}

float UShieldComp::ApplyShieldDamage(float Damage)
{
	// 쉴드가 꺼져있으면 데미지를 모두 통과
	if (!bShieldActive || CurrentShield <= 0.f)
		return Damage;

	CurrentShield -= Damage;
	float RemainingDamage = 0.f;

	// 쉴드 파괴 계산
	if (CurrentShield <= 0.f)
	{
		RemainingDamage = -CurrentShield;
		CurrentShield = 0.f;

		DeactivateShield();
		OnShieldBroken.Broadcast();
	}
	else
	{
		RemainingDamage = 0.f;
	}

	return RemainingDamage;
}

void UShieldComp::StartShieldCoolDown()
{
	bCanShield = false;
	GetWorld()->GetTimerManager().ClearTimer(ShieldCoolDownTimer);

	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Shield Cooldown Started"));
	}
	GetWorld()->GetTimerManager().SetTimer(
		ShieldCoolDownTimer,
		this,
		&UShieldComp::EndShieldCoolDown,
		ShieldDuration,
		false
	);
}

void UShieldComp::EndShieldCoolDown()
{
	/*
	if(GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Shield Cooldown Ended"));
	*/
	bCanShield = true;
}