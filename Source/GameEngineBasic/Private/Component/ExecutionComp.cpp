// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/ExecutionComp.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "Component/ExecutionComp.h"
#include "Camera/CameraComponent.h"
#include "EnemyShieldComponent.h"
#include "GameEngineBasic/Components/public/HealthComp.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UExecutionComp::UExecutionComp()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UExecutionComp::BeginPlay()
{
	Super::BeginPlay();
}

bool UExecutionComp::StartExecution()
{
	AActor* Target = FindExecutionTarget();
	if (!Target)
		return false;
	
	// 슬로우 모션
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.25f);

	// 알림 (Enemy, Player 둘 다 들을 수 있음)
	OnExecutionStart.Broadcast(Target);
	// 타이머에 Target 인자로 넘기기
	FTimerDelegate TimerDel;
	TimerDel.BindUObject(this, &UExecutionComp::EndExecution, Target);

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_ExecutionFinish,
		TimerDel,
		1.f,
		false
	);
	return true;
}

void UExecutionComp::EndExecution(AActor* Target)
{
	// 타이머 제거(안전)
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ExecutionFinish);
	
	// Give Damage to Enemy
	if (UHealthComp* EnemyHealth = Target->FindComponentByClass<UHealthComp>())
	{
		EnemyHealth->ApplyHealthDamage(999999);
	}
	
	// restore world time
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	
	OnExecutionEnd.Broadcast(Target);
}

AActor* UExecutionComp::FindExecutionTarget() const
{
	ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
	UCameraComponent* Camera = OwnerChar->FindComponentByClass<UCameraComponent>();
	if (!OwnerChar || !Camera) return nullptr;

	FVector Start = Camera->GetComponentLocation();
	FVector End = Start + Camera->GetForwardVector() * ExecutionRange;

	if (bDebugDraw)
	{
		UKismetSystemLibrary::DrawDebugLine(
			this, Start, End,
			FLinearColor::Red, 2.f, 1.f
		);
	}

	FHitResult Hit;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerChar);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ExecutionTraceChannel,
		Params
	);
	if (!bHit) return nullptr;

	AActor* Enemy = Hit.GetActor();
	if (!Enemy) return nullptr;
	
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow,
		FString::Printf(TEXT("Execution Target Found: %s"), *Enemy->GetName())
	);
	
	// EnemyShieldComponent üũ �� bCanExecuted �ʿ�
	if (auto* Shield = Enemy->FindComponentByClass<UEnemyShieldComponent>())
	{
		if (Shield->CanBeExecuted())
		{
			return Enemy;
		}
		
	}

	return nullptr;
}
