#include "MyPlayerHUD.h"
#include "Components/ProgressBar.h"
#include "ArcGaugeWidget.h"

#include "TargetingSystemComponent.h"
#include "EnemyShieldComponent.h"
#include "EnemyHuman.h"

#include "Kismet/GameplayStatics.h"

void UMyPlayerHUD::NativeConstruct()
{
	Super::NativeConstruct();

	CacheReferences();

	APawn* P = GetOwningPlayerPawn();
	if (!P) return;

	// Ammo Delegate
	if (UShooterComp* Shooter = P->FindComponentByClass<UShooterComp>())
	{
		Shooter->OnAmmoChanged.AddDynamic(this, &UMyPlayerHUD::HandleAmmoChanged);
	}

	// Energy도 Shooter 또는 EnergyComp에서 Bind
	// Shooter->OnEnergyChanged.AddDynamic(...)

	// Target 변경 Delegate
	if (TargetingComp)
	{
		TargetingComp->OnTargetChanged.AddDynamic(this, &UMyPlayerHUD::HandleTargetChanged);
	}

	// 시작 시 숨김 + 완전 투명
	EnemyUIOpacity = 0.f;
	TargetEnemyUIOpacity = 0.f;

	if (EnemyHealthGauge)
	{
		EnemyHealthGauge->SetRenderOpacity(0.f);
		EnemyHealthGauge->SetVisibility(ESlateVisibility::Hidden);
	}

	if (EnemyStunGauge)
	{
		EnemyStunGauge->SetRenderOpacity(0.f);
		EnemyStunGauge->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMyPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	/* --------------------------
		 HP 보간
	--------------------------*/
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

	/* --------------------------
		Fade 보간 (Opacity)
	--------------------------*/
	EnemyUIOpacity = FMath::FInterpTo(
		EnemyUIOpacity,
		TargetEnemyUIOpacity,
		InDeltaTime,
		EnemyUIFadeSpeed
	);

	if (EnemyHealthGauge)
		EnemyHealthGauge->SetRenderOpacity(EnemyUIOpacity);
	if (EnemyStunGauge)
		EnemyStunGauge->SetRenderOpacity(EnemyUIOpacity);
}


/*-----------------------------------
	    Target Switched
-----------------------------------*/
void UMyPlayerHUD::HandleTargetChanged(AEnemyHuman* NewTarget)
{
	/* 기존 Delegate 정리 */
	if (BoundEnemyHealthComp)
	{
		BoundEnemyHealthComp->OnHealthChanged_Ver2.RemoveDynamic(
			this, &UMyPlayerHUD::HandleEnemyHealthChanged);
		BoundEnemyHealthComp = nullptr;
	}

	if (BoundEnemyShieldComp)
	{
		BoundEnemyShieldComp->OnShieldDamaged.RemoveAll(this);
		BoundEnemyShieldComp->OnShieldRestored.RemoveAll(this);
		BoundEnemyShieldComp = nullptr;
	}

	/*----------------------------
	         Target 없음
	----------------------------*/
	if (!NewTarget)
	{
		TargetEnemyUIOpacity = 0.f;

		// FadeOut 끝나면 숨김 처리
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			[this]()
			{
				if (EnemyUIOpacity <= 0.01f)
				{
					EnemyHealthGauge->SetVisibility(ESlateVisibility::Hidden);
					EnemyStunGauge->SetVisibility(ESlateVisibility::Hidden);
				}
			},
			0.2f,
			false
		);
		return;
	}

	/*----------------------------
	        Target 있음 → FadeIn
	----------------------------*/
	TargetEnemyUIOpacity = 1.f;

	EnemyHealthGauge->SetVisibility(ESlateVisibility::Visible);
	EnemyStunGauge->SetVisibility(ESlateVisibility::Visible);

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

		BoundEnemyHealthComp->OnHealthChanged_Ver2.AddDynamic(
			this, &UMyPlayerHUD::HandleEnemyHealthChanged);
	}

	/*----------------------------
	         Stun Setup
	----------------------------*/
	BoundEnemyShieldComp = NewTarget->FindComponentByClass<UEnemyShieldComponent>();
	if (BoundEnemyShieldComp)
	{
		float Cur = BoundEnemyShieldComp->GetCurrentShield();
		float Max = BoundEnemyShieldComp->GetMaxShield();
		CurrentStunRatio = (Max > 0.f) ? Cur / Max : 0.f;

		EnemyStunGauge->SetPercent(CurrentStunRatio);

		BoundEnemyShieldComp->OnShieldDamaged.AddDynamic(
			this, &UMyPlayerHUD::HandleEnemyShieldDamaged);

		BoundEnemyShieldComp->OnShieldRestored.AddDynamic(
			this, &UMyPlayerHUD::HandleEnemyShieldRestored);
	}
}


/*-----------------------------------
	          HP 변화
-----------------------------------*/
void UMyPlayerHUD::HandleEnemyHealthChanged(float NewHealth, float MaxHealth)
{
	TargetEnemyHPRatio = (MaxHealth > 0.f) ? (NewHealth / MaxHealth) : 0.f;
}

/*-----------------------------------
	        Stun / Shield 변화
-----------------------------------*/
void UMyPlayerHUD::HandleEnemyShieldDamaged()
{
	if (!BoundEnemyShieldComp) return;

	float Cur = BoundEnemyShieldComp->GetCurrentShield();
	float Max = BoundEnemyShieldComp->GetMaxShield();
	CurrentStunRatio = (Max > 0.f) ? Cur / Max : 0.f;

	EnemyStunGauge->SetPercent(CurrentStunRatio);
}

void UMyPlayerHUD::HandleEnemyShieldRestored()
{
	CurrentStunRatio = 1.f;
	EnemyStunGauge->SetPercent(1.f);
}

/*-----------------------------------
	       Ammo / Energy
-----------------------------------*/
void UMyPlayerHUD::HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (AmmoGauge)
		AmmoGauge->SetPercent((float)CurrentAmmo / MaxAmmo);
}

void UMyPlayerHUD::HandleEnergyChanged(float CurrentEN, float MaxEN)
{
	if (EnergyGauge)
		EnergyGauge->SetPercent(CurrentEN / MaxEN);
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
