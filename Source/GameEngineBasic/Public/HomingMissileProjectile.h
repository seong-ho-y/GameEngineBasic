#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "HomingMissileProjectile.generated.h"

UENUM(BlueprintType)
enum class EHomingType : uint8
{
	ArcHoming UMETA(DisplayName = "Arc Homing"),     // 위로 올렸다가 유도
	DirectHoming UMETA(DisplayName = "Direct Homing") // 직선 유도 + 중력
};

UCLASS()
class GAMEENGINEBASIC_API AHomingMissileProjectile : public AProjectile
{
	GENERATED_BODY()

public:
	AHomingMissileProjectile();

protected:
	virtual void BeginPlay() override;
	void InitializeHomingMode();
	virtual void Tick(float DeltaSeconds) override;

public:
	void SetHomingTarget(AActor* Target);
	void SetHomingType(EHomingType NewType);

protected:
	void Explode();
	virtual void OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	void EnableGravity();

private:
	/** Homing type (Arc / Direct) */
	EHomingType HomingType = EHomingType::DirectHoming;

	/** Target to track */
	UPROPERTY()
	AActor* HomingTarget = nullptr;

	/** 내부 상태 */
	bool bIsArcPhase = false;
	float CachedStartZ = 0.f;

	/** 타이머 */
	FTimerHandle ExplosionTimerHandle;
	FTimerHandle GravityTimerHandle;

public:
	/** 기본 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float HomingSpeed = 2500.f;

	/** 유도 회전 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float TurnInterpSpeed = 5.0f;

	/** 아크 높이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float ArcHeight = 800.f;

	/** 중력 켜기 딜레이 (Direct용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float GravityEnableDelay = 0.8f;

	/** 폭발 딜레이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float ExplosionDelay = 3.0f;

	// 폭발 VFX
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
	UNiagaraSystem* ExplosionVfx;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* ExplosionSound;
};
