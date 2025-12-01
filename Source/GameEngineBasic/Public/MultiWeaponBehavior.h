#pragma once

#include "CoreMinimal.h"
#include "WeaponBehavior.h"
#include "MultiWeaponBehavior.generated.h"

UCLASS()
class GAMEENGINEBASIC_API UMultiWeaponBehavior : public UWeaponBehavior
{
	GENERATED_BODY()

public:

	virtual void OnFirePressed_Implementation() override;
	virtual void OnFireReleased_Implementation() override;

protected:

	int32 SpreadCount = 1;
	float SpreadAngle = 5.f;

	FTimerHandle AutoFireTimerHandle;

	/** Primary 구조 유지 */
	void FireOnce();

	/** 산탄 다발 생성 */
	void SpawnExtraProjectiles();

	/** 산탄 방향 */
	FVector GetSpreadDirection(const FVector& BaseDir) const;
};
