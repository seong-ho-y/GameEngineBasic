#include "MyPlayerState.h"
#include "InventoryComponent.h"

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
