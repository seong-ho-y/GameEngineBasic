// InventoryComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

class UPlayerStatsComponent;

UENUM(BlueprintType)
enum class EPartSlot : uint8
{
	Core,
	Upper,
	Lower
};

USTRUCT(BlueprintType)
struct FPartData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPartSlot Slot = EPartSlot::Core;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShieldBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeedBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostUseBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostRegenBonus = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WeightBonus = 0.f;
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
	UInventoryComponent();
	// 무기처럼 Part도 슬롯별로 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<EPartSlot, FName> EquippedParts;

	// 파츠 DataTable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parts")
	UDataTable* PartTable;

	virtual void BeginPlay() override;

	// ================================ 장착 ================================
	// 파츠
	UPROPERTY(VisibleAnywhere)
	FEquippedParts Parts;

	// 무기
	UPROPERTY(VisibleAnywhere)
	FEquippedWeapon Weapon;

	// ================================ 잠금 해제 이벤트 바인딩 ================================
	UFUNCTION()
	void OnWeaponUnlocked(FName WeaponRowName);

	UFUNCTION()
	void OnPartUnlocked(FName PartRowName);

	// ================================ 장착 가능 여부 확인 ================================
	UFUNCTION(BlueprintCallable)
	bool IsWeaponEquippable(FName WeaponRowName) const;

	UFUNCTION(BlueprintCallable)
	bool IsPartEquippable(FName PartRowName) const;

public:
	void EquipWeapon(FName RowName);
	void EquipPart(EPartSlot Slot, FName RowName);

	FName GetCurrentWeapon() const { return Weapon.WeaponRow; }
	FName GetPart(EPartSlot Slot) const;
	const FPartData* GetPartData(EPartSlot Slot) const;

	UPROPERTY()
	UPlayerStatsComponent* CachedStats;
};
