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

protected:

	int32 SpreadCount = 1;
	float SpreadAngle = 5.f;

	void SpawnMultiProjectiles();

	FVector MakeSpreadDirection(const FVector& BaseDir) const;
};
