// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/AI/Public/BTT_Hovering.h"

#include "AIController.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Pawn.h"
#include "Navigation/PathFollowingComponent.h"

UBTT_Hovering::UBTT_Hovering()
{
    NodeName = TEXT("Hovering(Warp)");
}

EBTNodeResult::Type UBTT_Hovering::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::ExecuteTask(OwnerComp, NodeMemory);

    // 1. AI 컨트롤러와 Pawn 가져오기
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (AIController == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (ControlledPawn == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    // 2. 현재 위치를 중심으로 랜덤 좌표 생성
    const FVector Origin = ControlledPawn->GetActorLocation();
    // 2. 현재 위치를 중심으로 랜덤 좌표 생성 (수학 계산)
        
    // 2-1. 3D 공간에서 랜덤한 '방향'을 구합니다. (길이가 1인 벡터)
    const FVector RandomDirection = FMath::VRand().GetSafeNormal();
    
    // 2-2. Origin으로부터 랜덤한 '거리'를 정합니다. (너무 가깝지 않게 최소값 설정 가능)
    const float RandomDistance = FMath::FRandRange(500.0f, HoverRadius);
    
    // 2-3. 최종 목표 지점 = 시작점 + 방향 * 거리
    const FVector TargetLocation = Origin + RandomDirection * RandomDistance;

    // 3. VFX 소환
    if (WarpVFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WarpVFX, ControlledPawn->GetActorLocation());
    }

    // 4. 계산된 '새로운 랜덤 위치'로 AI 이동 명령 (순간이동)
    ControlledPawn->SetActorLocation(TargetLocation);

    // 태스크가 성공적으로 완료되었음을 알립니다.
    return EBTNodeResult::Succeeded;
}