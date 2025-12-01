// InventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class EPartSlot : uint8
{
	Core,
	Upper,
	Lower
};

// 플레이어 파츠 저장용
USTRUCT(BlueprintType)
struct FEquippedParts
{
	GENERATED_BODY()

	UPROPERTY() FName CorePart;
	UPROPERTY() FName UpperPart;
	UPROPERTY() FName LowerPart;
};

// 플레이어 무기 저장용
USTRUCT(BlueprintType)
struct FEquippedWeapon
{
	GENERATED_BODY()

	UPROPERTY() FName WeaponRow;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEENGINEBASIC_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 파츠
	UPROPERTY(VisibleAnywhere)
	FEquippedParts Parts;

	// 무기
	UPROPERTY(VisibleAnywhere)
	FEquippedWeapon Weapon;

public:
	void EquipWeapon(FName RowName);
	void EquipPart(EPartSlot Slot, FName RowName);

	FName GetCurrentWeapon() const { return Weapon.WeaponRow; }
	FName GetPart(EPartSlot Slot) const;
};
