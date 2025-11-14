#pragma once

#include "CoreMinimal.h"
#include "HomingMissileProjectile.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "BTT_Shoot.generated.h"

USTRUCT(BlueprintType)
struct FShootOption
{
	GENERATED_BODY()
	
	// ====== 타이밍 / 연사 ======
	UPROPERTY(EditAnywhere, Category = "Shoot|Timing")
	float ShootDelay = 2.f;
	
	UPROPERTY(EditAnywhere, Category="Shoot|Timing")
	int32 BurstCount = 1;

	UPROPERTY(EditAnywhere, Category="Shoot|Timing")
	float BurstInterval = 0.15f;

	// ====== 조준 / 퍼짐 ======
	UPROPERTY(EditAnywhere, Category="Shoot|Spread")
	int32 SpreadCount = 1;

	UPROPERTY(EditAnywhere, Category="Shoot|Spread")
	float SpreadAngle = 3.0f;

	// ====== 랜덤 타겟 오프셋 ======
	UPROPERTY(EditAnywhere, Category="Shoot|Offset")
	bool bUseRandomTargetOffset = false;

	UPROPERTY(EditAnywhere, Category="Shoot|Offset")
	float RandomAreaRadius = 300.0f;

	// ====== 예측 사격 ======
	UPROPERTY(EditAnywhere, Category="Shoot|Predictive")
	bool bUsePredictive = false;

	UPROPERTY(EditAnywhere, Category="Shoot|Predictive")
	float ProjectileSpeed = 2500.0f;

	// ====== 호밍 모드 ======
	UPROPERTY(EditAnywhere, Category = "Shoot|Homing")
	bool ArcHoming = true;

};

UCLASS()
class GAMEENGINEBASIC_API UBTT_Shoot : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_Shoot();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void StartFire(UBehaviorTreeComponent* OwnerComp, APawn* Self, UShooterComp* ShooterComp, AActor* Target, int32 CurrentBurstIndex);
	void ClearTimers(APawn* Self);
	
public:
	// 블랙보드 키 (플레이어 등)
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	// 총구 이름
	UPROPERTY(EditAnywhere, Category="Shoot")
	FName MuzzleName = "Muzzle";
	
	UPROPERTY(EditAnywhere, Category="Shoot|Weapon")
	EProjectileType ProjectileType = EProjectileType::Rifle;
	
	// 사격 옵션
	UPROPERTY(EditAnywhere, Category="Shoot")
	FShootOption ShootOption;

private:
	// 🔹 멤버 핸들로 타이머 추적
	FTimerHandle BurstTimerHandle;
	FTimerHandle DelayTimerHandle;
};
