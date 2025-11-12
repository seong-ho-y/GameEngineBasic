// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameFramework/Actor.h"
#include <sstream>

// Sets default values for this component's properties
UHealthComp::UHealthComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UHealthComp::BeginPlay()
{
	Super::BeginPlay();
	InitStats();

	// 시작 시 재생 타이머는 비활성 상태로 둠
	NextRegenTime = -FLT_MAX;
	LastDamageTime = -FLT_MAX;
}

void UHealthComp::InitStats()
{
	CurrentHealth = MaxHealth;
	CurrentShield = (bUseShield ? MaxShield : 0);
	BroadcastChanged();
}

// Called every frame
void UHealthComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 쉴드가 가득 차 있지 않다면, 마지막 피격으로부터 Delay가 지날 때마다 +1
	if (CurrentShield < MaxShield)
	{
		const float Now = GetWorld()->GetTimeSeconds();

		// 아직 재생이 시작되지 않았다면 (피격 후 대기시간 경과 시점 설정)
		if (NextRegenTime == -FLT_MAX && LastDamageTime != -FLT_MAX)
		{
			NextRegenTime = LastDamageTime + ShieldRegenDelay;
		}

		// 딱 "ShieldRegenDelay가 지날 때마다 +1" 이산 회복
		while (NextRegenTime != -FLT_MAX && Now >= NextRegenTime && CurrentShield < MaxShield)
		{
			RestoreShield(1);
			NextRegenTime += ShieldRegenDelay; // 다음 틱 예약
		}
	}
	else
	{
		// 가득 찼으면 재생 타이머 리셋
		NextRegenTime = -FLT_MAX;
	}
}

bool UHealthComp::IsInvincible() const
{
	if (!bUseInvincibleFrame) return false;
	float Now = GetWorld()->GetTimeSeconds();
	return (Now - LastDamageTime < InvincibleDuration);
}

void UHealthComp::TakeDamage(int32 DamageAmount)
{
	if (DamageAmount <= 0) return;

	// 무적 체크
	if (IsInvincible())
		return;

	LastDamageTime = GetWorld()->GetTimeSeconds();
	OnDamageTaken.Broadcast(GetOwner());

	// 쉴드 우선
	if (bUseShield && CurrentShield > 0)
	{
		ApplyShieldDamage(DamageAmount);
	}
	else
	{
		ApplyHealthDamage(DamageAmount);
	}

	// 쉴드 회복 시작 예약
	if (bUseShieldRegen && bUseShield)
	{
		StopShieldRegenTimer();
		GetWorld()->GetTimerManager().SetTimer(
			ShieldRegenTimerHandle,
			this,
			&UHealthComp::StartShieldRegenTimer,
			ShieldRegenDelay,
			false
		);
	}

	// 디버그
	if (bDebugHealthLog && GEngine)
	{
		std::stringstream ss;
		ss << "HP:" << CurrentHealth << " SHIELD:" << CurrentShield;
		GEngine->AddOnScreenDebugMessage(97, 1.5f, FColor::Cyan, ss.str().c_str());
	}
}
void UHealthComp::StartShieldRegenTimer()
{
	if (!bUseShieldRegen || !bUseShield) return;
	if (CurrentShield >= MaxShield) return;

	GetWorld()->GetTimerManager().SetTimer(
		ShieldRegenTimerHandle,
		[this]()
		{
			if (CurrentShield >= MaxShield)
			{
				StopShieldRegenTimer();
				return;
			}
			RestoreShield(1);
		},
		ShieldRegenDelay,
		true
	);
}

void UHealthComp::StopShieldRegenTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(ShieldRegenTimerHandle);
}
void UHealthComp::RestoreShield(int Amount)
{
	if (!bUseShield || Amount <= 0) return;

	int32 OldShield = CurrentShield;
	CurrentShield = FMath::Clamp(CurrentShield + Amount, 0, MaxShield);

	if (CurrentShield >= MaxShield)
		StopShieldRegenTimer();

	if (CurrentShield != OldShield)
		BroadcastChanged();
}

void UHealthComp::ApplyShieldDamage(int Amount)
{
	if (!bUseShield || Amount <= 0 || CurrentShield <= 0) return;

	CurrentShield = FMath::Clamp(CurrentShield - Amount, 0, MaxShield);

	if (CurrentShield == 0)
		OnShieldBroken.Broadcast(GetOwner());

	BroadcastChanged();
}

void UHealthComp::ApplyHealthDamage(float Amount)
{
	if (Amount <= 0 || CurrentHealth <= 0) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0, MaxHealth);

	if (CurrentHealth == 0)
		OnDeath.Broadcast(GetOwner());

	UE_LOG(LogTemp, Warning, TEXT("[Enemy] Health Take Damaged : %f | Current Health : %f"),Amount, CurrentHealth);
	BroadcastChanged();
}

void UHealthComp::BroadcastChanged() const
{
	OnHealthChanged.Broadcast(GetOwner(), CurrentHealth, CurrentShield);
}
