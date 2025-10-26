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

void UHealthComp::InitStats()
{
	CurrentHealth = MaxHealth;
	CurrentShield = MaxShield;
	BroadcastChanged();
}

void UHealthComp::TakeDamage()
{
	// 피격 시각 갱신 및 재생 시작점 재설정

	const float Now = GetWorld()->GetTimeSeconds();
	const float InvincibleDuration = 0.3f; // 무적시간 (초)

	if (Now - LastDamageTime < InvincibleDuration)
	{
		// 아직 무적시간 중이라면 데미지 무시
		return;
	}

	float Elapsed = Now - LastDamageTime;
	float RemainingInvincible = FMath::Max(0.f, InvincibleDuration - Elapsed);

	// 남은 무적시간 출력
	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("무적 남은 시간: %.2f초"), RemainingInvincible);
		GEngine->AddOnScreenDebugMessage(97, 1.0f, FColor::Yellow, Msg);
	}


	LastDamageTime = Now;
	NextRegenTime = Now + ShieldRegenDelay;

	// 우선 실드 1 감소, 실드가 없으면 체력 1 감소
	if (CurrentShield > 0)
	{
		ApplyShieldDamage(1);
	}
	else
	{
		ApplyHealthDamage(1);
	}
	std::stringstream ss;

	ss << "CurrentShield: " << CurrentShield << ' ' << "CurrentHealth: " << CurrentHealth;
	if (GEngine) GEngine->AddOnScreenDebugMessage(95, 1.0f, FColor::Blue, ss.str().c_str());

}

void UHealthComp::RestoreShield(int Amount)
{
	if (Amount <= 0) return;

	const int OldShield = CurrentShield;
	CurrentShield = FMath::Clamp(CurrentShield + Amount, 0, MaxShield);

	// 가득 차면 재생 타이머는 멈춤
	if (CurrentShield >= MaxShield)
	{
		NextRegenTime = -FLT_MAX;
	}

	// 변경 브로드캐스트
	if (CurrentShield != OldShield)
	{
		BroadcastChanged();
	}
}

void UHealthComp::ApplyShieldDamage(int Amount)
{
	if (Amount <= 0 || CurrentShield <= 0) return;

	CurrentShield = FMath::Clamp(CurrentShield - Amount, 0, MaxShield);

	if (CurrentShield == 0)
	{
		OnShieldBroken.Broadcast(GetOwner());
	}


	BroadcastChanged();
}

void UHealthComp::ApplyHealthDamage(int Amount)
{
	if (Amount <= 0 || CurrentHealth <= 0) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0, MaxHealth);

	if (CurrentHealth == 0)
	{
		OnDeath.Broadcast(GetOwner());
	}

	BroadcastChanged();
}

void UHealthComp::BroadcastChanged()
{
	OnHealthChanged.Broadcast(GetOwner(), (float)CurrentHealth, (float)CurrentShield);
}