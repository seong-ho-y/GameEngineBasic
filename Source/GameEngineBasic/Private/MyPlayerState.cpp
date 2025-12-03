#include "MyPlayerState.h"
#include "InventoryComponent.h"
#include "Item/AbilityUnlockItem.h"

AMyPlayerState::AMyPlayerState()
{
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}

void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (!Inventory) return;

	// 🔹 기본 무기 장착
	Inventory->EquipWeapon("HandgunBasic");

	// 🔹 기본 파츠 장착
	Inventory->EquipPart(EPartSlot::Core,  "C_Base");
	Inventory->EquipPart(EPartSlot::Upper, "Up_Base");
	Inventory->EquipPart(EPartSlot::Lower, "Low_Base");
}

void AMyPlayerState::UnlockWeapon(FName WeaponRowName)
{
	if (UnlockStatus.UnlockedWeapons.Contains(WeaponRowName))
		return;

	UnlockStatus.UnlockedWeapons.Add(WeaponRowName);

	// 인벤토리에 언락 정보 반영 가능 (필요 시)
	if (Inventory)
	{
		Inventory->OnWeaponUnlocked(WeaponRowName);
	}

	// UI 브로드캐스트는 Character 또는 Inventory 쪽에서 추가 가능
	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Weapon Unlocked: %s"), *WeaponRowName.ToString());
}

void AMyPlayerState::UnlockPart(FName PartRowName)
{
	if (UnlockStatus.UnlockedParts.Contains(PartRowName))
		return;

	UnlockStatus.UnlockedParts.Add(PartRowName);

	if (Inventory)
	{
		Inventory->OnPartUnlocked(PartRowName);
	}

	UE_LOG(LogTemp, Log, TEXT("[PlayerState] Part Unlocked: %s"), *PartRowName.ToString());
}

bool AMyPlayerState::IsWeaponUnlocked(FName WeaponRowName) const
{
	return UnlockStatus.UnlockedWeapons.Contains(WeaponRowName);
}

bool AMyPlayerState::IsPartUnlocked(FName PartRowName) const
{
	return UnlockStatus.UnlockedParts.Contains(PartRowName);
}

void AMyPlayerState::UnlockAbility(EAbilityType Ability)
{
	switch (Ability)
	{
	case EAbilityType::Sprint: AbilityStatus.bCanSprint = true; break;
	case EAbilityType::Dash:   AbilityStatus.bCanDash = true; break;
	case EAbilityType::Flying: AbilityStatus.bCanFly = true; break;
	case EAbilityType::Shield: AbilityStatus.bCanShield = true; break;
	case EAbilityType::Boost: AbilityStatus.bCanDash = true; break;
	}

	UE_LOG(LogTemp, Warning, TEXT("[PlayerState] Ability Unlocked: %d"), (int)Ability);
}

bool AMyPlayerState::CanUseAbility(EAbilityType Ability) const
{

	switch (Ability)
	{
	case EAbilityType::Sprint:
		return AbilityStatus.bCanSprint;

	case EAbilityType::Dash:
		return AbilityStatus.bCanDash;

	case EAbilityType::Flying:
		return AbilityStatus.bCanFly;

	case EAbilityType::Shield:
		return AbilityStatus.bCanShield;

	case EAbilityType::Boost:
		return AbilityStatus.bCanBoost;
	}

	return false;

}
