// Fill out your copyright notice in the Description page of Project Settings.


#include "TargetingSystemComponent.h"

#include "EngineUtils.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Interfaces/IHttpResponse.h"

class UMyPlayerHUD;
// Sets default values for this component's properties
UTargetingSystemComponent::UTargetingSystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

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
    GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Magenta,
    FString::Printf(TEXT("Viewport: %d × %d"),
        GEngine->GameViewport->Viewport->GetSizeXY().X,
        GEngine->GameViewport->Viewport->GetSizeXY().Y
    ));
    
	GEngine->AddOnScreenDebugMessage(-9, 0.f, FColor::Black, TEXT("Ticking..."));
	AEnemyHuman* NewTarget = FindClosestEnemyInView();

	if (NewTarget != CurrentTarget)
	{
		GEngine->AddOnScreenDebugMessage(-17, 5.f, FColor::Emerald, TEXT("CurrentTarget Updated"));
		CurrentTarget = NewTarget;
		OnTargetChanged.Broadcast(CurrentTarget); // Notify to HUD
	}
}

AEnemyHuman* UTargetingSystemComponent::FindClosestEnemyInView()
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!IsValid(OwnerPawn)) return nullptr;

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!IsValid(PC)) return nullptr;

    // 1) 뷰포트 크기
    int32 ViewportX = 0, ViewportY = 0;
    PC->GetViewportSize(ViewportX, ViewportY);

    // 2) 원 중심 = 화면 중심
    const FVector2D CircleCenter(
        ViewportX * 0.5f,
        ViewportY * 0.5f
    );

    // 3) 원 반지름 = 화면 세로 기준 15%
    float Radius = ViewportY * 0.15f;

    // 디버그용
    GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan,
        FString::Printf(TEXT("[Circle] Center=(%.1f, %.1f) R=%.1f"),
            CircleCenter.X, CircleCenter.Y, Radius));

    AEnemyHuman* Closest = nullptr;
    float ClosestDist = FLT_MAX;

    for (TActorIterator<AEnemyHuman> It(World); It; ++It)
    {
        AEnemyHuman* Enemy = *It;
        if (!IsValid(Enemy)) continue;

        // 🔹 2) 적 위치를 화면 좌표로 변환
        FVector2D EnemyScreenPos;
        if (!UGameplayStatics::ProjectWorldToScreen(PC, Enemy->GetActorLocation(), EnemyScreenPos))
            continue;

        // 디버그
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White,
            FString::Printf(TEXT("[Enemy %s] ScreenPos=(%.1f, %.1f)"),
                *Enemy->GetName(), EnemyScreenPos.X, EnemyScreenPos.Y));

        // 🔹 3) 원 안에 들어왔는지만 검사
        const float DistToCenter = FVector2D::Distance(EnemyScreenPos, CircleCenter);

        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue,
            FString::Printf(TEXT("[Enemy %s] DistToCenter=%.1f (R=%.1f)"),
                *Enemy->GetName(), DistToCenter, Radius));

        if (DistToCenter > Radius)
            continue; // 원 밖 → 후보 제외

        // 🔹 4) 가장 중앙에 가까운 적 선택
        if (DistToCenter < ClosestDist)
        {
            ClosestDist = DistToCenter;
            Closest = Enemy;
        }
    }

    if (!Closest)
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red,
            TEXT("[Targeting] No enemy inside circle"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Green,
            FString::Printf(TEXT("[Targeting] Final Target = %s"),
                *Closest->GetName()));
    }

    return Closest;
}
