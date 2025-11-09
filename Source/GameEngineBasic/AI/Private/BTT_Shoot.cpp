#include "../Public/BTT_Shoot.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

UBTT_Shoot::UBTT_Shoot()
{
	NodeName = TEXT("Shoot");
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

	USkeletalMeshComponent* Mesh = Self->FindComponentByClass<USkeletalMeshComponent>();
	FVector MuzzleLoc = Mesh ? Mesh->GetSocketLocation(MuzzleName) : Self->GetActorLocation();

	// 패턴 랜덤 선택
	if (ShootPattern == EShootPattern::RandomPattern)
	{
		// AreaRandom~Volley 포함해서 랜덤 (원하면 범위 조절)
		ShootPattern = static_cast<EShootPattern>(FMath::RandRange(0, static_cast<int32>(EShootPattern::Volley)));
	}

	// 기본 조준
	const FVector BaseDir = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();
	ShooterComp->SetFireDirection(BaseDir);

	bool bUsingTimer = false;

	switch (ShootPattern)
	{
	case EShootPattern::Single:
		bUsingTimer = FireSingleType(Self, ShooterComp, MuzzleLoc, Target, OwnerComp);
		break;

	case EShootPattern::Predictive:
		bUsingTimer = FirePredictiveType(Self, ShooterComp, MuzzleLoc, Target, OwnerComp);
		break;

	case EShootPattern::Spread:
		bUsingTimer = FireSpreadType(Self, ShooterComp, MuzzleLoc, Target, OwnerComp);
		break;

	case EShootPattern::AreaRandom:
		// 여기에도 원하면 BurstCount 적용 가능하지만, 기본은 즉시형 포격
		FireAreaRandom(ShooterComp, MuzzleLoc, Target);
		// PostFireDelay 적용 원하면 아래 두 줄 사용:
		if (PostFireDelay > 0.f)
		{
			StartPostFireDelay(OwnerComp, Self);
			return EBTNodeResult::InProgress;
		}
		break;

	case EShootPattern::Sweep:
		bUsingTimer = FireSweep(Self, ShooterComp, MuzzleLoc, Target, OwnerComp);
		break;

	case EShootPattern::Volley:
		bUsingTimer = FireVolley(Self, ShooterComp, MuzzleLoc, Target, OwnerComp);
		break;

	default:
		break;
	}

	return bUsingTimer ? EBTNodeResult::InProgress : EBTNodeResult::Succeeded;
}

//
// 공통 헬퍼: 마지막 샷 이후 PostFireDelay 처리
//
void UBTT_Shoot::StartPostFireDelay(UBehaviorTreeComponent& OwnerComp, APawn* Self) const
{
	if (!Self || PostFireDelay <= 0.f)
	{
		// 쿨다운 없음: 즉시 종료
		const_cast<UBTT_Shoot*>(this)->FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	FTimerHandle Handle;
	Self->GetWorldTimerManager().SetTimer(
		Handle,
		FTimerDelegate::CreateLambda([this, &OwnerComp]()
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}),
		PostFireDelay,
		false
	);
}

//
// Single / Predictive / Spread: BurstCount / BurstInterval / PostFireDelay 적용
//

bool UBTT_Shoot::FireSingleType(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp)
{
	if (!Self || !ShooterComp) return false;

	const int32 Count = FMath::Max(BurstCount, 1);
	const bool bUseBurst = (Count > 1);
	const bool bUseCooldown = (PostFireDelay > 0.f);

	// 완전 즉발형 (단발 + 쿨다운 없음)
	if (!bUseBurst && !bUseCooldown)
	{
		ShooterComp->TryFire();
		return false;
	}

	// Burst 또는 쿨다운이 있으면 타이머 기반 비동기 처리
	for (int32 i = 0; i < Count; ++i)
	{
		const float Delay = i * BurstInterval;

		FTimerHandle Handle;
		Self->GetWorldTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([this, &OwnerComp, ShooterComp, Self, i, Count]()
			{
				if (ShooterComp)
				{
					ShooterComp->TryFire();
				}

				if (i == Count - 1)
				{
					StartPostFireDelay(OwnerComp, Self);
				}
			}),
			Delay,
			false
		);
	}

	return true;
}

bool UBTT_Shoot::FirePredictiveType(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp)
{
	if (!Self || !ShooterComp || !Target) return false;

	const int32 Count = FMath::Max(BurstCount, 1);
	const bool bUseBurst = (Count > 1);
	const bool bUseCooldown = (PostFireDelay > 0.f);

	if (!bUseBurst && !bUseCooldown)
	{
		// 즉시 1발 예측샷
		FVector TargetVel = Target->GetVelocity();
		const float Distance = FVector::Dist(MuzzleLoc, Target->GetActorLocation());
		const float TimeToHit = Distance / ProjectileSpeed;
		const FVector Predicted = Target->GetActorLocation() + TargetVel * TimeToHit;

		const FVector Dir = (Predicted - MuzzleLoc).GetSafeNormal();
		ShooterComp->SetFireDirection(Dir);
		ShooterComp->TryFire();
		return false;
	}

	// Burst/쿨다운 포함 예측샷
	for (int32 i = 0; i < Count; ++i)
	{
		const float Delay = i * BurstInterval;

		FTimerHandle Handle;
		Self->GetWorldTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([this, &OwnerComp, ShooterComp, Self, Target, MuzzleLoc, i, Count]()
			{
				if (ShooterComp && Target)
				{
					const FVector TargetVel = Target->GetVelocity();
					const float Distance = FVector::Dist(MuzzleLoc, Target->GetActorLocation());
					const float TimeToHit = Distance / ProjectileSpeed;
					const FVector Predicted = Target->GetActorLocation() + TargetVel * TimeToHit;

					const FVector Dir = (Predicted - MuzzleLoc).GetSafeNormal();
					ShooterComp->SetFireDirection(Dir);
					ShooterComp->TryFire();
				}

				if (i == Count - 1)
				{
					StartPostFireDelay(OwnerComp, Self);
				}
			}),
			Delay,
			false
		);
	}

	return true;
}

bool UBTT_Shoot::FireSpreadType(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp)
{
	if (!Self || !ShooterComp || !Target) return false;

	const int32 Count = FMath::Max(BurstCount, 1);
	const bool bUseBurst = (Count > 1);
	const bool bUseCooldown = (PostFireDelay > 0.f);

	if (!bUseBurst && !bUseCooldown)
	{
		// 즉시 1회 스프레드(SpreadCount발)
		const FVector BaseDir = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();
		const FRotator BaseRot = BaseDir.Rotation();

		for (int32 s = 0; s < SpreadCount; ++s)
		{
			FRotator R = BaseRot;
			R.Pitch += FMath::FRandRange(-SpreadAngle, SpreadAngle);
			R.Yaw   += FMath::FRandRange(-SpreadAngle, SpreadAngle);

			const FVector Dir = R.Vector();
			ShooterComp->SetFireDirection(Dir);
			ShooterComp->TryFire();
		}
		return false;
	}

	// Burst/쿨다운 포함 스프레드
	for (int32 i = 0; i < Count; ++i)
	{
		const float Delay = i * BurstInterval;

		FTimerHandle Handle;
		Self->GetWorldTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([this, &OwnerComp, ShooterComp, Self, Target, MuzzleLoc, i, Count]()
			{
				if (ShooterComp && Target)
				{
					const FVector BaseDir = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();
					const FRotator BaseRot = BaseDir.Rotation();

					for (int32 s = 0; s < SpreadCount; ++s)
					{
						FRotator R = BaseRot;
						R.Pitch += FMath::FRandRange(-SpreadAngle, SpreadAngle);
						R.Yaw   += FMath::FRandRange(-SpreadAngle, SpreadAngle);

						const FVector Dir = R.Vector();
						ShooterComp->SetFireDirection(Dir);
						ShooterComp->TryFire();
					}
				}

				if (i == Count - 1)
				{
					StartPostFireDelay(OwnerComp, Self);
				}
			}),
			Delay,
			false
		);
	}

	return true;
}

//
// AreaRandom: 즉시 포격 1회 (원하면 여기에도 BurstCount 적용 가능)
//

void UBTT_Shoot::FireAreaRandom(UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target)
{
	if (!ShooterComp || !Target) return;

	const FVector Center = Target->GetActorLocation();
	const FVector RandomPoint = Center + FVector(
		FMath::FRandRange(-AreaRadius, AreaRadius),
		FMath::FRandRange(-AreaRadius, AreaRadius),
		FMath::FRandRange(-AreaRadius * 0.3f, AreaRadius * 0.3f)
	);

	const FVector Dir = (RandomPoint - MuzzleLoc).GetSafeNormal();
	ShooterComp->SetFireDirection(Dir);
	ShooterComp->TryFire();
}

//
// Sweep: 일정 각도 범위로 훑으면서 발사
//

bool UBTT_Shoot::FireSweep(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp)
{
	if (!Self || !ShooterComp || !Target) return false;

	const FVector BaseDir = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();
	const FRotator BaseRot = BaseDir.Rotation();

	const int32 Steps = FMath::Max(SweepSteps, 1);
	const float StepAngle = (Steps > 1) ? (SweepAngle / (Steps - 1)) : 0.f;
	const float StepInterval = BurstInterval > 0.f ? BurstInterval : 0.08f;

	for (int32 i = 0; i < Steps; ++i)
	{
		const float Offset = -SweepAngle * 0.5f + StepAngle * i;
		const float Delay = i * StepInterval;

		FTimerHandle Handle;
		Self->GetWorldTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([this, &OwnerComp, ShooterComp, Self, BaseRot, Offset, i, Steps]()
			{
				if (ShooterComp)
				{
					FRotator Rot = BaseRot;
					Rot.Yaw += Offset;

					const FVector Dir = Rot.Vector();
					ShooterComp->SetFireDirection(Dir);
					ShooterComp->TryFire();
				}

				if (i == Steps - 1)
				{
					StartPostFireDelay(OwnerComp, Self);
				}
			}),
			Delay,
			false
		);
	}

	return true;
}

//
// Volley: 다발 포격 / 미사일
//

bool UBTT_Shoot::FireVolley(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp)
{
	if (!Self || !ShooterComp || !Target) return false;

	const int32 Count = FMath::Max(VolleyCount, 1);

	for (int32 i = 0; i < Count; ++i)
	{
		const float Delay = i * VolleyInterval;

		FTimerHandle Handle;
		Self->GetWorldTimerManager().SetTimer(
			Handle,
			FTimerDelegate::CreateLambda([this, &OwnerComp, ShooterComp, Self, Target, MuzzleLoc, i, Count]()
			{
				if (ShooterComp && Target)
				{
					const FVector Forward = (Target->GetActorLocation() - MuzzleLoc).GetSafeNormal();
					FRotator Rot = Forward.Rotation();
					Rot.Yaw   += FMath::FRandRange(-6.f, 6.f);
					Rot.Pitch += FMath::FRandRange(-3.f, 3.f);

					const FVector Dir = Rot.Vector();
					ShooterComp->SetFireDirection(Dir);
					ShooterComp->TryFire();
				}

				if (i == Count - 1)
				{
					StartPostFireDelay(OwnerComp, Self);
				}
			}),
			Delay,
			false
		);
	}

	return true;
}
