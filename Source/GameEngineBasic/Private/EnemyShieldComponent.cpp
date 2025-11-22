// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyShieldComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UEnemyShieldComponent::UEnemyShieldComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UEnemyShieldComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UEnemyShieldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bIsShieldBroken) return;

	TimeSinceLastDamage += DeltaTime;
	if (TimeSinceLastDamage >= RecoverDelay) // Check RegenerateTime
	{
		RegenerateShield(DeltaTime);
	}
	// ...
}

/** 데미지 처리 로직 */
float UEnemyShieldComponent::ApplyDamage(float Damage)
{
	TimeSinceLastDamage = 0.f; // 회복 딜레이 리셋

	// 이미 쉴드 파괴된 상태면 그대로 통과
	if (bIsShieldBroken)
		return Damage;

	// 쉴드가 데미지 일부 흡수
	float Absorbed = Damage * DamageReduction;
	CurrentShield = FMath::Clamp(CurrentShield - Absorbed, 0.f, MaxShield);
	OnShieldDamaged.Broadcast();

	// 완전히 파괴됨
	if (CurrentShield <= 0.f)
	{
		UE_LOG(LogTemp,Log,TEXT("Shield Destroyed"));
		bIsShieldBroken = true;
		bCanExecuted = true;
		ShieldBrokenVFX();
		OnShieldBreak.Broadcast();

		// 일정 시간 후 Execution 해제 && 적 쉴드 복구
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_ExecutionReset, [this]()
		{
			bCanExecuted = false;
			RestoreShieldFull(); //쉴드 전체 회복
		}, ExecutionTime, false);
	}
	UE_LOG(LogTemp, Log, TEXT("[Enemy] Shield TakeDamaged : %f | Current Shield : %f"), Damage, CurrentShield)
	// 남은 데미지는 체력으로 넘김
	return Damage - Absorbed;
}

/** 쉴드 회복 로직 */
void UEnemyShieldComponent::RegenerateShield(float DeltaTime)
{
	if (CurrentShield < MaxShield)
	{
		CurrentShield = FMath::Min(CurrentShield + (RecoverSpeed * DeltaTime), MaxShield);
		if (CurrentShield == MaxShield)
		{
			OnShieldRestored.Broadcast();
		}
	}
}

// 쉴드 완전 복구 
void UEnemyShieldComponent::RestoreShieldFull()
{
	CurrentShield = MaxShield;
	bIsShieldBroken = false;
	bCanExecuted = false;
	ShieldResotreVFX();
	OnShieldRestored.Broadcast();
}

void UEnemyShieldComponent::ShieldBrokenVFX()
{
	if (ShieldBrokenVfx)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),ShieldBrokenVfx,GetOwner()->GetActorLocation());
	if (ShieldBrokenParticle)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShieldBrokenParticle, GetOwner()->GetActorLocation());
	if (ShieldBrokenSound)
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(),ShieldBrokenSound, GetOwner()->GetActorLocation());
}
void UEnemyShieldComponent::ShieldResotreVFX()
{
	if (ShieldResotreVfx)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ShieldResotreVfx, GetOwner()->GetActorLocation());
	if (ShieldRestoreParticle)
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ShieldRestoreParticle, GetOwner()->GetActorLocation());
	if (ShieldRestoreSound)
		UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ShieldRestoreSound, GetOwner()->GetActorLocation());
}
