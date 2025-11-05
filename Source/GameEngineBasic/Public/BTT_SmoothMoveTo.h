#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SmoothMoveTo.generated.h"
/**
 * CharacterMovement 전용 부드러운 MoveTo
 * - AddMovementInput 기반 가속/감속
 * - 브레이크 거리(v^2 / 2a)로 도착 반경 내에서 자연 감속
 * - Velocity 직접 수정/즉시정지(StopMovementImmediately) 사용하지 않음
 */
UCLASS()
class GAMEENGINEBASIC_API UBTT_SmoothMoveTo : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_SmoothMoveTo();

	/** 목표 Actor (우선 사용) */
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetActorKey;

	/** 목표 위치 (Actor 없을 때 사용) */
	UPROPERTY(EditAnywhere, Category="Blackboard")
	FBlackboardKeySelector TargetLocationKey;

	/** 목표에 가까워졌을 때 작업 실패로 처리할지 여부 (false면 그냥 성공 처리) */
	UPROPERTY(EditAnywhere, Category="Behavior")
	bool bFailIfNoTarget = true;

	/** 최대 이동 속도 (단위: cm/s). 캐릭터의 MaxWalkSpeed와 다르게, 여기선 '목표 스피드 상한'으로만 사용 */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="1.0"))
	float MaxSpeed = 600.f;

	/** 가속률 (cm/s^2). FInterpConstantTo 속도 변화량으로 사용 */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="1.0"))
	float Accel = 1200.f;

	/** 감속률 (cm/s^2). 브레이크 거리 계산 및 FInterpConstantTo에 사용 */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="1.0"))
	float Decel = 1600.f;

	/** 도착 판정 반경 (cm) */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="0.0"))
	float AcceptanceRadius = 120.f;

	/** 도착 시 최종 속도 허용 임계치 (cm/s). 이 값 이하가 되면 성공 종료 */
	UPROPERTY(EditAnywhere, Category="Movement", meta=(ClampMin="0.0"))
	float StopSpeedThreshold = 5.f;

	/** 속도 방향으로 회전할지 여부 */
	UPROPERTY(EditAnywhere, Category="Rotation")
	bool bRotateToVelocity = true;

	/** Yaw 보간 속도 (1/s) */
	UPROPERTY(EditAnywhere, Category="Rotation", meta=(ClampMin="0.0"))
	float YawInterpSpeed = 6.f;

	/** 2D 이동(수평 기준) vs 3D 이동(공중 포함). 보통 지상 적은 false(2D) */
	UPROPERTY(EditAnywhere, Category="Movement")
	bool bUse3DDirection = false;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** 블랙보드에서 목표 위치를 꺼내기 (Actor 우선, 없으면 Vector) */
	bool GetTargetLocation(UBehaviorTreeComponent& OwnerComp, FVector& OutLocation) const;

	/** 목표 속도(DesiredSpeed)로 부드럽게 근접시키는 가/감속 적용 */
	float ApplyAcceleration(float CurrentSpeed, float DesiredSpeed, float DeltaSeconds) const;

	/** 현재 거리와 속도를 바탕으로 브레이크 거리 기반 목표 속도 산출 */
	float ComputeDesiredSpeed(float DistanceToTarget, float CurrentSpeed) const;

private:
	/** 내부 사용: 2D/3D 방향 계산 */
	void ComputeDirectionDistance(const FVector& From, const FVector& To, FVector& OutDir, float& OutDistance) const;
};