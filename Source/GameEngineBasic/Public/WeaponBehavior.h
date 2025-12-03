#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WeaponBehavior.generated.h"

class UWeaponComponent;

UCLASS(Abstract, Blueprintable)
class GAMEENGINEBASIC_API UWeaponBehavior : public UObject
{
	GENERATED_BODY()
	
public:

	UPROPERTY()
	UWeaponComponent* OwnerWeapon;

	virtual void Initialize(UWeaponComponent* InWeapon)
	{
		OwnerWeapon = InWeapon;
	}

	// 마우스 눌렀을 때
	UFUNCTION(BlueprintNativeEvent)
	void OnFirePressed();
	virtual void OnFirePressed_Implementation() {}

	// 마우스 뗐을 때
	UFUNCTION(BlueprintNativeEvent)
	void OnFireReleased();
	virtual void OnFireReleased_Implementation() {}
};