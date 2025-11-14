// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetingSystemComponent.h"

#include "EngineUtils.h"
#include "Interfaces/IHttpResponse.h"

// Sets default values for this component's properties
UTargetingSystemComponent::UTargetingSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTargetingSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTargetingSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	CurrentTarget = FindClosestEnemyInView();
}
AEnemyHuman* UTargetingSystemComponent::FindClosestEnemyInView()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) return nullptr;

	APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
	if (!PC) return nullptr;

	FVector CamLoc;
	FRotator CamRot;
	PC->GetPlayerViewPoint(CamLoc, CamRot);
	FVector CanForward = CamRot.Vector();

	AEnemyHuman* Closest = nullptr;
	float ClosestScreenDist = FLT_MAX;

	for (TActorIterator<AEnemyHuman> It(GetWorld()); It; ++It)
	{
		AEnemyHuman* Enemy = *It;
		if (!Enemy || Enemy->IsPendingKillPending()) continue;

		// 인식 최대 거리 설정
		FVector ToEnemy = Enemy->GetActorLocation() - CamLoc;
		float Dist = ToEnemy.Size();
		if (Dist > MaxDistance) continue;

		// 시야각 체크
		float Dot = FVector::DotProduct(CanForward, ToEnemy.GetSafeNormal());
		float AngleDeg = FMath::RadiansToDegrees(acosf(Dot));
		if (AngleDeg > ViewAngle) continue;

		// 화면 중심 기준 거리
		FVector2D ScreenPos;
		FVector2D ScreenCenter;
		if (!UGameplayStatics::ProjectWorldToScreen(PC,Enemy->GetActorLocation(), ScreenPos)) continue;
		{
			int32 SX, SY;
			PC->GetViewportSize(SX,SY);
			ScreenCenter = FVector2D(SX / 2, SY / 2);
		}

		float ScreenDist = FVector2D::Distance(ScreenCenter, ScreenPos);
		if (ScreenDist < ClosestScreenDist)
		{
			// 시야 차폐 라인 트레이스
			FHitResult Hit;
			GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, Enemy->GetActorLocation(), ECC_Visibility);
			if (Hit.GetActor() != Enemy) continue;

			ClosestScreenDist = ScreenDist;
			Closest = Enemy;
		}
	}

	return Closest;
}

