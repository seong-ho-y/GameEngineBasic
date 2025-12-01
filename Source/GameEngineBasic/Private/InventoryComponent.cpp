// InventoryComponent.cpp
#include "InventoryComponent.h"
#include "SpaceCharacter/SpaceCharacter.h"


UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UInventoryComponent::EquipPart(EPartSlot Slot, FName PartRowName)
{
	EquippedParts.Add(Slot, PartRowName);
	
}

FName UInventoryComponent::GetPart(EPartSlot Slot) const
{
	if (const FName* Found = EquippedParts.Find(Slot))
	{
		return *Found;
	}
	return NAME_None;
}

const FPartData* UInventoryComponent::GetPartData(EPartSlot Slot) const
{
	if (!PartTable) return nullptr;

	FName RowName = GetPart(Slot);
	if (RowName == NAME_None) return nullptr;

	return PartTable->FindRow<FPartData>(RowName, TEXT("GetPartData"));
}

void UInventoryComponent::EquipWeapon(FName RowName)
{
	Weapon.WeaponRow = RowName;
}