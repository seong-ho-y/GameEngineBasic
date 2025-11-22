// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/ExecutionComp.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "Camera/CameraComponent.h"
#include "EnemyShieldComponent.h"

#include "GameFramework/Character.h"
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

	// 상태 브로드캐스트
	OnExecutionStart.Broadcast(Target);
	return true;
}

void UExecutionComp::EndExecution()
{
	AActor* Target = FindExecutionTarget();
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
	
	// EnemyShieldComponent 체크 → bCanExecuted 필요
	if (auto* Shield = Enemy->FindComponentByClass<UEnemyShieldComponent>())
	{
		if (Shield->CanBeExecuted())
		{
			return Enemy;
		}
		
	}

	return nullptr;
}
