#include "../Public/BTT_Shoot.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"

UBTT_Shoot::UBTT_Shoot()
{
    NodeName = TEXT("Shoot (Unified)");
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTT_Shoot::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AI = OwnerComp.GetAIOwner();
    APawn* Self = AI ? AI->GetPawn() : nullptr;
    if (!Self) return EBTNodeResult::Failed;

    UShooterComp* ShooterComp = Self->FindComponentByClass<UShooterComp>();
    if (!ShooterComp) return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AActor* Target = BB ? Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
    if (!Target) return EBTNodeResult::Failed;
    // 호밍 타입 지정
    ShooterComp->bUseArcHoming = ShootOption.ArcHoming;
    
    ShooterComp->CurrentTarget = Target;
    ShooterComp->CurrentProjectileType = ProjectileType;

    // 🔸 기존 타이머 클리어
    ClearTimers(Self);

    // 첫 번째 사격 시작
    StartFire(&OwnerComp, Self, ShooterComp, Target, 0);

    return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTT_Shoot::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::AbortTask(OwnerComp, NodeMemory);

    AAIController* AI = OwnerComp.GetAIOwner();
    APawn* Self = AI ? AI->GetPawn() : nullptr;
    if (Self)
        ClearTimers(Self);

    return EBTNodeResult::Aborted;
}

void UBTT_Shoot::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);

    AAIController* AI = OwnerComp.GetAIOwner();
    APawn* Self = AI ? AI->GetPawn() : nullptr;
    if (Self)
        ClearTimers(Self);
}

void UBTT_Shoot::ClearTimers(APawn* Self)
{
    if (!Self) return;
    FTimerManager& TM = Self->GetWorldTimerManager();
    TM.ClearTimer(BurstTimerHandle);
    TM.ClearTimer(DelayTimerHandle);
}

void UBTT_Shoot::StartFire(UBehaviorTreeComponent* OwnerComp, APawn* Self, UShooterComp* ShooterComp, AActor* Target, int32 CurrentBurstIndex)
{
    if (!ShooterComp || !Target || !OwnerComp || !Self)
        return;
    
    
    // 🔸 방향 계산
    USkeletalMeshComponent* Mesh = Self->FindComponentByClass<USkeletalMeshComponent>();
    const FVector MuzzleLoc = Mesh ? Mesh->GetSocketLocation(MuzzleName) : Self->GetActorLocation();
    ShooterComp->MuzzleSocketName = MuzzleName;
    FVector BaseDir = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();

    // 예측 사격
    if (ShootOption.bUsePredictive)
    {
        const FVector TargetVel = Target->GetVelocity();
        const float Distance = FVector::Dist(MuzzleLoc, Target->GetActorLocation());
        const float TimeToHit = Distance / FMath::Max(ShootOption.ProjectileSpeed, 1.0f);
        const FVector PredictedLoc = Target->GetActorLocation() + TargetVel * TimeToHit;
        BaseDir = (PredictedLoc - MuzzleLoc).GetSafeNormal();
    }

    // 랜덤 오프셋
    if (ShootOption.bUseRandomTargetOffset)
    {
        const FVector RandomOffset(
            FMath::FRandRange(-ShootOption.RandomAreaRadius, ShootOption.RandomAreaRadius),
            FMath::FRandRange(-ShootOption.RandomAreaRadius, ShootOption.RandomAreaRadius),
            FMath::FRandRange(-ShootOption.RandomAreaRadius * 0.3f, ShootOption.RandomAreaRadius * 0.3f)
        );
        BaseDir = (Target->GetActorLocation() + RandomOffset - MuzzleLoc).GetSafeNormal();
    }

    // Spread 처리
    FRotator SpreadRot = BaseDir.Rotation();
    if (ShootOption.SpreadCount > 1)
    {
        SpreadRot.Yaw += FMath::FRandRange(-ShootOption.SpreadAngle, ShootOption.SpreadAngle);
        SpreadRot.Pitch += FMath::FRandRange(-ShootOption.SpreadAngle, ShootOption.SpreadAngle);
    }

    const FVector FinalDir = SpreadRot.Vector();
    ShooterComp->SetFireDirection(FinalDir);

    // 🔹 실제 발사
    const bool bFired = ShooterComp->TryFire();
    //UE_LOG(LogTemp, Warning, TEXT("Burst[%d] Fired=%d"), CurrentBurstIndex, bFired);

    // 🔹 다음 Burst 예약
    if (CurrentBurstIndex + 1 < ShootOption.BurstCount)
    {
        const int32 NextIndex = CurrentBurstIndex + 1;

        FTimerDelegate NextFireDelegate;
        NextFireDelegate.BindLambda([this, OwnerComp, Self, ShooterComp, Target, NextIndex]()
        {
            StartFire(OwnerComp, Self, ShooterComp, Target, NextIndex);
        });

        Self->GetWorldTimerManager().ClearTimer(BurstTimerHandle);
        Self->GetWorldTimerManager().SetTimer(
            BurstTimerHandle,
            NextFireDelegate,
            ShootOption.BurstInterval,
            false);
    }
    else
    {
        // 🔹 Burst 끝 → Delay 후 Task 종료
        if (ShootOption.ShootDelay > 0.f)
        {
            FTimerDelegate EndDelegate;
            EndDelegate.BindLambda([this, OwnerComp]()
            {
                if (OwnerComp)
                    FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
            });

            Self->GetWorldTimerManager().ClearTimer(DelayTimerHandle);
            Self->GetWorldTimerManager().SetTimer(
                DelayTimerHandle,
                EndDelegate,
                ShootOption.ShootDelay,
                false);
        }
        else
        {
            FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}
