#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTT_Shoot.generated.h"

class UShooterComp;
class APawn;
class AActor;

UENUM(BlueprintType)
enum class EShootPattern : uint8
{
	Single,        // 기본 직사
	Predictive,    // 예측 사격
	Spread,        // 산탄/스프레드
	AreaRandom,    // 플레이어 주변 랜덤 포격
	Sweep,         // 각도 스윕 사격
	Volley,        // 포문/미사일 연속 발사
	RandomPattern  // 위 타입들 중 랜덤 선택
};

UCLASS()
class GAMEENGINEBASIC_API UBTT_Shoot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_Shoot();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 단일/예측/스프레드 타입에 공통으로 Burst & Cooldown 적용하는 함수들.
	 *  true 리턴 시: 타이머 기반(InProgress), false 리턴 시: 즉시 종료(Succeeded).
	 */
	bool FireSingleType(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp);
	bool FirePredictiveType(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp);
	bool FireSpreadType(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp);

	// 나머지 패턴 (이 내부에서 PostFireDelay까지 처리)
	void FireAreaRandom(UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target);
	bool FireSweep(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp);
	bool FireVolley(APawn* Self, UShooterComp* ShooterComp, const FVector& MuzzleLoc, AActor* Target, UBehaviorTreeComponent& OwnerComp);

	// Burst & 쿨다운 헬퍼
	void StartPostFireDelay(UBehaviorTreeComponent& OwnerComp, APawn* Self) const;

protected:
	// 공통 설정
	UPROPERTY(EditAnywhere, Category = "Shoot")
	FName MuzzleName = "Muzzle";

	UPROPERTY(EditAnywhere, Category = "Shoot")
	EShootPattern ShootPattern = EShootPattern::Single;

	UPROPERTY(EditAnywhere, Category = "Shoot|Burst", meta=(ClampMin="1"))
	int32 BurstCount = 1; // 1=단발, 2이상=연발

	UPROPERTY(EditAnywhere, Category = "Shoot|Burst", meta=(ClampMin="0.0"))
	float BurstInterval = 0.12f; // Burst 발 사이 간격

	UPROPERTY(EditAnywhere, Category = "Shoot|Cooldown", meta=(ClampMin="0.0"))
	float PostFireDelay = 0.5f; // 마지막 탄 이후 다음 공격까지 대기 시간

	// Predictive
	UPROPERTY(EditAnywhere, Category = "Predictive", meta=(ClampMin="1.0"))
	float ProjectileSpeed = 3000.f;

	// Spread
	UPROPERTY(EditAnywhere, Category = "Spread", meta=(ClampMin="1"))
	int32 SpreadCount = 5;

	UPROPERTY(EditAnywhere, Category = "Spread", meta=(ClampMin="0.0"))
	float SpreadAngle = 4.f;

	// AreaRandom
	UPROPERTY(EditAnywhere, Category = "AreaRandom", meta=(ClampMin="0.0"))
	float AreaRadius = 600.f;

	// Sweep
	UPROPERTY(EditAnywhere, Category = "Sweep", meta=(ClampMin="0.0"))
	float SweepAngle = 20.f;

	UPROPERTY(EditAnywhere, Category = "Sweep", meta=(ClampMin="1"))
	int32 SweepSteps = 5;

	// Volley
	UPROPERTY(EditAnywhere, Category = "Volley", meta=(ClampMin="1"))
	int32 VolleyCount = 3;

	UPROPERTY(EditAnywhere, Category = "Volley", meta=(ClampMin="0.0"))
	float VolleyInterval = 0.25f;

	// 타겟
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
