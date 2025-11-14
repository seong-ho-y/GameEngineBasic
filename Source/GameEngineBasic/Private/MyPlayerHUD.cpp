// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerHUD.h"

void UMyPlayerHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!EnemyHealthBar) return;

	// 타겟 없으면 그냥 리턴 (HandleTargetChanged에서 숨김 처리함)
	if (!BoundEnemyHealthComp)
		return;

	const float InterpSpeed = 10.f; // 숫자 키우면 더 빨리 따라감
	DisplayEnemyHPRatio = FMath::FInterpTo(
		DisplayEnemyHPRatio,
		TargetEnemyHPRatio,
		InDeltaTime,
		InterpSpeed
	);

	EnemyHealthBar->SetPercent(DisplayEnemyHPRatio);
}
void UMyPlayerHUD::HandleTargetChanged(AEnemyHuman* NewTarget)
{
	// 이전 적의 Health 델리게이트 언바인딩
	if (BoundEnemyHealthComp)
	{
		BoundEnemyHealthComp->OnHealthChanged_Ver2.RemoveDynamic(this, &UMyPlayerHUD::HandleEnemyHealthChanged);
		BoundEnemyHealthComp = nullptr;
	}

	if (!NewTarget || !EnemyHealthBar)
	{
		// 타겟 없음 → UI 숨기기
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		TargetEnemyHPRatio = 0.f;
		return;
	}

	// 새 타겟의 HealthComp 찾기
	BoundEnemyHealthComp = NewTarget->FindComponentByClass<UHealthComp>();
	if (!BoundEnemyHealthComp)
	{
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
		TargetEnemyHPRatio = 0.f;
		return;
	}

	// 현재 체력 기준으로 초기화
	float Cur = BoundEnemyHealthComp->GetCurrentHealth();
	float Max = BoundEnemyHealthComp->GetMaxHealth();
	TargetEnemyHPRatio   = (Max > 0.f) ? Cur / Max : 0.f;
	DisplayEnemyHPRatio  = TargetEnemyHPRatio;

	EnemyHealthBar->SetPercent(DisplayEnemyHPRatio);
	EnemyHealthBar->SetVisibility(ESlateVisibility::Visible);

	// 체력 변화 델리게이트 바인딩
	BoundEnemyHealthComp->OnHealthChanged_Ver2.AddDynamic(this, &UMyPlayerHUD::HandleEnemyHealthChanged);
}

void UMyPlayerHUD::HandleEnemyHealthChanged(float NewHealth, float MaxHealth)
{
	TargetEnemyHPRatio = (MaxHealth > 0.f) ? NewHealth / MaxHealth : 0.f;
}

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
	if (EnemyHealthBar)
	{
		EnemyHealthBar->SetVisibility(ESlateVisibility::Hidden);
	}
	
}


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
		APawn* OwnerPawn = GetOwningPlayerPawn();
		if (OwnerPawn)
		{
			CachedTarget = OwnerPawn->FindComponentByClass<UTargetingSystemComponent>();
		}
	}
	return CachedTarget;
}
void UMyPlayerHUD::HandleAmmoChanged(int32 CurrentAmmo, int32 MaxAmmo)
{

	if (AmmoBar) // UProgressBar*
	{
		float Ratio = (float)CurrentAmmo / (float)MaxAmmo;
		AmmoBar->SetPercent(Ratio);
	}
}
void UMyPlayerHUD::HandleEnergyChanged(float CurrentEN, float MaxEN)
{
	float Percent = CurrentEN / MaxEN;
	FuelBar->SetPercent(Percent);
}