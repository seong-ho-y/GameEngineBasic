// InventoryComponent.cpp
#include "InventoryComponent.h"

void UInventoryComponent::EquipWeapon(FName RowName)
{
	Weapon.WeaponRow = RowName;
}

void UInventoryComponent::EquipPart(EPartSlot Slot, FName RowName)
{
	switch (Slot)
	{
	case EPartSlot::Core: Parts.CorePart = RowName; break;
	case EPartSlot::Upper: Parts.UpperPart = RowName; break;
	case EPartSlot::Lower: Parts.LowerPart = RowName; break;
	}
}

FName UInventoryComponent::GetPart(EPartSlot Slot) const
{
	switch (Slot)
	{
	case EPartSlot::Core: return Parts.CorePart;
	case EPartSlot::Upper: return Parts.UpperPart;
	case EPartSlot::Lower: return Parts.LowerPart;
	default: return NAME_None;
	}
}
