#include "MyPlayerState.h"
#include "InventoryComponent.h"

AMyPlayerState::AMyPlayerState()
{
	Inventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
}
void AMyPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (Inventory)
	{
		Inventory->EquipWeapon("HandgunBasic");
	}
}
