#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretLaserComponent.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class USkeletalMeshComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEENGINEBASIC_API UTurretLaserComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurretLaserComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime,
		enum ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

	/** 레이저를 켜거나 끄는 함수 */
	UFUNCTION(BlueprintCallable, Category="Turret|Laser")
	void SetLaserEnabled(bool bEnable);

	/** 현재 타겟(보통 플레이어) 설정 */
	UFUNCTION(BlueprintCallable, Category="Turret|Laser")
	void SetCurrentTarget(AActor* NewTarget);

	/** 기절 상태 변경 시 호출 (bStunned=true면 레이저 Off) */
	UFUNCTION(BlueprintCallable, Category="Turret|Laser")
	void OnStunStateChanged(bool bStunned);

protected:
	/** 나이아가라 이펙트 에셋 (Beam 타입) */
	UPROPERTY(EditAnywhere, Category="Turret|Laser")
	UNiagaraSystem* LaserSystem;

	/** 메시에 붙여질 소켓 이름 (총구/헤드 등) */
	UPROPERTY(EditAnywhere, Category="Turret|Laser")
	FName LaserStartSocketName = TEXT("bn_head"); // 필요하면 Muzzle 등으로 변경

	/** 최대 레이저 길이 (타겟 없거나 막혔을 때) */
	UPROPERTY(EditAnywhere, Category="Turret|Laser")
	float MaxLaserDistance = 4000.f;

	/** 너무 가까우면 조준선 끄기 위한 거리 */
	UPROPERTY(EditAnywhere, Category="Turret|Laser")
	float MinLaserDistance = 500.f;

	/** 나이아가라 컴포넌트 인스턴스 */
	UPROPERTY(Transient)
	UNiagaraComponent* LaserComp = nullptr;

	/** 현재 조준 중인 타겟 */
	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentTarget = nullptr;

	/** 외부에서 켬/끔 요청한 플래그 (BT/AI 등) */
	bool bWantsLaser = true;

	/** 기절 상태 여부 */
	bool bIsStunned = false;

	/** 실제로 지금 켜진 상태인지 */
	bool bLaserActive = false;

	/** 시작 위치/타겟 위치 계산 및 나이아가라 변수 업데이트 */
	void UpdateLaserFX(float DeltaTime);

	/** 나이아가라 켜기/끄기 실제 처리 */
	void ApplyLaserActiveState(bool bShouldBeActive);
};
