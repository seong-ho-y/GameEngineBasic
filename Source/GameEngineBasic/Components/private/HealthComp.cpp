// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/Components/public/HealthComp.h"
#include "Component/ShieldComp.h"
#include "GameFramework/Actor.h"

UHealthComp::UHealthComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UHealthComp::BeginPlay()
{
	Super::BeginPlay();
	InitStats();
	/*
	NextRegenTime = -FLT_MAX;
	LastDamageTime = -FLT_MAX;
	*/
}

void UHealthComp::InitStats()
{
	if (AActor* Owner = GetOwner())
	{
		ShieldComp = Owner->FindComponentByClass<UShieldComp>();
	}
	CurrentHealth = MaxHealth;
	CurrentHealth = MaxHealth;
	BroadcastStatus();
}

void UHealthComp::RestoreFullHealth()
{
	CurrentHealth = MaxHealth;
	BroadcastStatus();
}

float UHealthComp::ApplyHealthDamage(float Damage)
{
	if (bIsInvincible || Damage <= 0.f)
	{
		return 0.f;
	}

	StartInvincibility();
	float IncomingDamage = Damage;

	// 1) 쉴드가 있다면 → 먼저 처리
	if (ShieldComp && ShieldComp->IsShieldActive())
	{
		IncomingDamage = ShieldComp->ApplyShieldDamage(Damage);
	}

	// 2) 남은 데미지가 없으면 → 체력 감소 없음
	if (IncomingDamage <= 0.f)
	{
		BroadcastStatus();
		return 0.f;
	}

	float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - IncomingDamage, 0.f, MaxHealth);

	if (CurrentHealth <= 0.f)
	{
		HandleDeath();
	}

	BroadcastStatus();
	return PreviousHealth - CurrentHealth;
}

void UHealthComp::StartInvincibility()
{
	bIsInvincible = true;

	GetWorld()->GetTimerManager().ClearTimer(InvincibleTimerHandle);

	GetWorld()->GetTimerManager().SetTimer(
		InvincibleTimerHandle,
		this,
		&UHealthComp::EndInvincibility,
		InvincibleDuration,
		false
	);
}

void UHealthComp::EndInvincibility()
{
	bIsInvincible = false;
}


void UHealthComp::Heal(float Amount)
{
	if (Amount <= 0.f)
		return;

	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
	BroadcastStatus();
}

void UHealthComp::BroadcastStatus()
{
	OnHealthChanged.Broadcast(CurrentHealth);                
	OnHealthChanged_Ver2.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComp::HandleDeath()
{
	OnDeath.Broadcast(GetOwner());
}
