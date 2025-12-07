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
		// 2. ������Ʈ ��� �� ����
		ShieldCollision->RegisterComponent();
		ShieldCollision->AttachToComponent(Owner->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

		// 3. �ݸ��� ����
		ShieldCollision->InitSphereRadius(120.f); 
		ShieldCollision->SetCollisionProfileName(TEXT("ShieldActor")); 
		ShieldCollision->SetNotifyRigidBodyCollision(true);

		// 4. �̺�Ʈ ���ε�
		ShieldCollision->OnComponentHit.AddDynamic(this, &UShieldComp::OnShieldHit);

		// 5. �ʱ� ����: ���� (�浹 ����, ����)
		ShieldCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ShieldCollision->SetHiddenInGame(true);
	}
}

void UShieldComp::ActivateShield()
{
	if (!ShieldCollision) return;
	if (!bCanShield) return;
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
	// 1. �߻�ü Ȯ��
	AProjectile* Projectile = Cast<AProjectile>(OtherActor);
	if (!Projectile) return;

	// 2. ������ ����
	// Owner�� TakeDamage -> HealthComp -> ShieldComp::ApplyShieldDamage 
	// ������ ȣ��Ǿ� ���� �帧�� �����˴ϴ�.
	if (AActor* Owner = GetOwner())
	{
		float Damage = Projectile->DamageAmount;
		UGameplayStatics::ApplyDamage(Owner, Damage, nullptr, Owner, UDamageType::StaticClass());
	}

	// 3. �ǰ� ����Ʈ ���
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
	// ���尡 ���������� �������� ��� ���
	if (!bShieldActive || CurrentShield <= 0.f)
		return Damage;

	CurrentShield -= Damage;
	float RemainingDamage = 0.f;

	// ���� �ı� ���
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

	/*if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Shield Cooldown Started"));
	}
	*/
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