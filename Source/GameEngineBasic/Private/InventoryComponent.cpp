// InventoryComponent.cpp
#include "InventoryComponent.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "GameEngineBasic/Public/MyPlayerState.h"


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

	if (GEngine)
	{
		FString SlotName =
			(Slot == EPartSlot::Core)  ? TEXT("Core") :
			(Slot == EPartSlot::Upper) ? TEXT("Upper") :
										 TEXT("Lower");

		GEngine->AddOnScreenDebugMessage(
			-1, 3.f, FColor::Green,
			FString::Printf(TEXT("[EquipPart] Slot = %s | Row = %s"),
				*SlotName,
				*PartRowName.ToString())
		);
	}

	if (CachedStats)
	{
		CachedStats->ApplyParts();
	}
	else
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(
				-1, 3.f, FColor::Red,
				TEXT("[EquipPart] CachedStats == NULL (ApplyParts 호출 못함)"));
	}
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

void UInventoryComponent::OnWeaponUnlocked(FName WeaponRowName)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,FString::Printf(TEXT("[Inventory] Weapon Unlocked: %s"),*WeaponRowName.ToString()));
}

void UInventoryComponent::OnPartUnlocked(FName PartRowName)
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,FString::Printf(TEXT("[Inventory] Part Unlocked: %s"),*PartRowName.ToString()));
}

bool UInventoryComponent::IsWeaponEquippable(FName WeaponRowName) const
{
	ASpaceCharacter* OwnerChar = Cast<ASpaceCharacter>(GetOwner());
	if (!OwnerChar) return false;

	AMyPlayerState* PS = OwnerChar->GetPlayerState<AMyPlayerState>();
	if (!PS) return false;

	// 기본 무기는 항상 가능
	if (WeaponRowName == "HandgunBasic")
		return true;

	return PS->UnlockStatus.UnlockedWeapons.Contains(WeaponRowName);
}

bool UInventoryComponent::IsPartEquippable(FName PartRowName) const
{
	ASpaceCharacter* OwnerChar = Cast<ASpaceCharacter>(GetOwner());
	if (!OwnerChar) return false;

	AMyPlayerState* PS = OwnerChar->GetPlayerState<AMyPlayerState>();
	if (!PS) return false;

	// 기본 파츠는 항상 사용 가능
	if (PartRowName == "C_Base" ||
		PartRowName == "Up_Base" ||
		PartRowName == "Low_Base")
		return true;

	return PS->UnlockStatus.UnlockedParts.Contains(PartRowName);
}