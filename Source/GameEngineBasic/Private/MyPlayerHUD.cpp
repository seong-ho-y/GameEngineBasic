// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerHUD.h"


void UMyPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	CacheReferences();
	
	APawn* P = GetOwningPlayerPawn();
	if (!P) return;

	UShooterComp* Shooter = P->FindComponentByClass<UShooterComp>();
	if (!Shooter) return;

	Shooter->OnAmmoChanged.AddDynamic(this, &UMyPlayerHUD::HandleAmmoChanged);
	
	if (TargetingComp)
	{
		TargetingComp->OnTargetChanged.AddDynamic(this, &UMyPlayerHUD::HandleTargetChanged);
	}

	// 시작할 땐 적 없으니까 숨겨두기
	if (EnemyHealthGauge) EnemyHealthGauge->SetVisibility(ESlateVisibility::Hidden);
	if (EnemyStunGauge)   EnemyStunGauge->SetVisibility(ESlateVisibility::Hidden);
}

void UMyPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (EnemyHealthGauge && BoundEnemyHealthComp)
	{
		DisplayEnemyHPRatio = FMath::FInterpTo(
			DisplayEnemyHPRatio,
			TargetEnemyHPRatio,
			InDeltaTime,
			10.f
		);

		EnemyHealthGauge->SetPercent(DisplayEnemyHPRatio);
	}

	// Stun updates instantly or with small interpolation if needed
	if (EnemyStunGauge && BoundEnemyShieldComp)
	{
		EnemyStunGauge->SetPercent(CurrentStunRatio);
	}
}

void UMyPlayerHUD::HandleTargetChanged(AEnemyHuman* NewTarget)
{
	// 이전 적의 Health 델리게이트 언바인딩
	if (BoundEnemyHealthComp)
	{
		BoundEnemyHealthComp->OnHealthChanged_Ver2.RemoveDynamic(this, &UMyPlayerHUD::HandleEnemyHealthChanged);
		BoundEnemyHealthComp = nullptr;
	}
	if (BoundEnemyShieldComp)
	{
		BoundEnemyShieldComp->OnShieldDamaged.RemoveAll(this);
		BoundEnemyShieldComp->OnShieldRestored.RemoveAll(this);
		BoundEnemyShieldComp = nullptr;
	}
	if (!NewTarget)
	{
		EnemyHealthGauge->SetVisibility(ESlateVisibility::Hidden);
		EnemyStunGauge->SetVisibility(ESlateVisibility::Hidden);
		return;
	}
	/*----------------------------
			   HP Setup
		----------------------------*/
	BoundEnemyHealthComp = NewTarget->FindComponentByClass<UHealthComp>();
	if (BoundEnemyHealthComp)
	{
		float Cur = BoundEnemyHealthComp->GetCurrentHealth();
		float Max = BoundEnemyHealthComp->GetMaxHealth();

		TargetEnemyHPRatio  = (Max > 0.f) ? Cur / Max : 0.f;
		DisplayEnemyHPRatio = TargetEnemyHPRatio;

		EnemyHealthGauge->SetPercent(DisplayEnemyHPRatio);
		EnemyHealthGauge->SetVisibility(ESlateVisibility::Visible);

		BoundEnemyHealthComp->OnHealthChanged_Ver2.AddDynamic(
			this, &UMyPlayerHUD::HandleEnemyHealthChanged);
	}

	/*----------------------------
		  Shield/Stun Setup
	----------------------------*/
	BoundEnemyShieldComp = NewTarget->FindComponentByClass<UEnemyShieldComponent>();
	if (BoundEnemyShieldComp)
	{
		float Cur = BoundEnemyShieldComp->GetCurrentShield();
		float Max = BoundEnemyShieldComp->GetMaxShield();
		CurrentStunRatio = (Max > 0.f) ? Cur / Max : 0.f;

		EnemyStunGauge->SetPercent(CurrentStunRatio);
		EnemyStunGauge->SetVisibility(ESlateVisibility::Visible);

		BoundEnemyShieldComp->OnShieldDamaged.AddDynamic(
			this, &UMyPlayerHUD::HandleEnemyShieldDamaged);
		BoundEnemyShieldComp->OnShieldRestored.AddDynamic(
			this, &UMyPlayerHUD::HandleEnemyShieldRestored);
	}
}

/*-----------------------------------
			 HP Changed
-----------------------------------*/
void UMyPlayerHUD::HandleEnemyHealthChanged(float NewHealth, float MaxHealth)
{
	TargetEnemyHPRatio = (MaxHealth > 0.f) ? (NewHealth / MaxHealth) : 0.f;
}

/*-----------------------------------
		 Stun / Shield Changed
-----------------------------------*/
void UMyPlayerHUD::HandleEnemyShieldDamaged()
{
	if (!BoundEnemyShieldComp) return;

	float Cur = BoundEnemyShieldComp->GetCurrentShield();
	float Max = BoundEnemyShieldComp->GetMaxShield();
	CurrentStunRatio = (Max > 0.f) ? Cur / Max : 0.f;

	if (EnemyStunGauge)
		EnemyStunGauge->SetPercent(CurrentStunRatio);
}

void UMyPlayerHUD::HandleEnemyShieldRestored()
{
	CurrentStunRatio = 1.f;
	if (EnemyStunGauge)
		EnemyStunGauge->SetPercent(1.f);
}

/*-----------------------------------
		 Ammo / Energy
-----------------------------------*/
void UMyPlayerHUD::HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	float Ratio = (float)CurrentAmmo / (float)MaxAmmo;
	if (AmmoGauge)
		AmmoGauge->SetPercent(Ratio);
}

void UMyPlayerHUD::HandleEnergyChanged(float CurrentEN, float MaxEN)
{
	float Ratio = CurrentEN / MaxEN;
	if (EnergyGauge)
		EnergyGauge->SetPercent(Ratio);
}

/*-----------------------------------
	 Reference Caching
-----------------------------------*/
void UMyPlayerHUD::CacheReferences()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		OwningPawn = PC->GetPawn();
		if (OwningPawn)
		{
			TargetingComp = OwningPawn->FindComponentByClass<UTargetingSystemComponent>();
		}
	}
}

UTargetingSystemComponent* UMyPlayerHUD::GetTargetingComp()
{
	if (!CachedTarget)
	{
		if (APawn* OwnerPawn = GetOwningPlayerPawn())
		{
			CachedTarget = OwnerPawn->FindComponentByClass<UTargetingSystemComponent>();
		}
	}
	return CachedTarget;
}