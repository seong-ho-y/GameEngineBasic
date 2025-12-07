#include "PlayerStatsComponent.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "MyPlayerState.h"
#include "InventoryComponent.h"

// 게임플레이 관련
#include "Component/FuelComponent.h"
#include "Component/ShieldComp.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPlayerStatsComponent::ApplyParts()
{
	ASpaceCharacter* OwnerChar = Cast<ASpaceCharacter>(GetOwner());
	if (!OwnerChar)
	{
		/*if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
				TEXT("[StatsComp] ERROR: Owner is not ASpaceCharacter"));
		*/
		return;
	}

	AMyPlayerState* PS = OwnerChar->GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->Inventory)
	{
		/*if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
				TEXT("[StatsComp] WARNING: Inventory not found"));
		*/
		return;
	}

	UInventoryComponent* Inv = PS->Inventory;
	if (!Inv->PartTable)
	{	/*
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				TEXT("[StatsComp] WARNING: PartTable is NULL"));
		*/
	}

	//-------------------------------------------------------
	// FinalStats 기본 초기화
	//-------------------------------------------------------
	FinalStats = BaseStats;
	/*
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White,
			TEXT("[ApplyParts] Reset FinalStats to BaseStats"));
			*/
	//-------------------------------------------------------
	// 파츠 3종 적용
	//-------------------------------------------------------
	const EPartSlot Slots[3] =
	{
		EPartSlot::Core,
		EPartSlot::Upper,
		EPartSlot::Lower
	};

	for (EPartSlot Slot : Slots)
	{
		const FPartData* Part = Inv->GetPartData(Slot);
		if (!Part)
		{
			continue;
		}

		// Slot 이름 출력
		FString SlotStr =
			(Slot == EPartSlot::Core)  ? TEXT("Core") :
			(Slot == EPartSlot::Upper) ? TEXT("Upper") :
										 TEXT("Lower");
		

		// 스탯 적용
		FinalStats.MaxHealth  += Part->HealthBonus;
		FinalStats.MaxShield  += Part->ShieldBonus;
		FinalStats.MoveSpeed  += Part->MoveSpeedBonus;
		FinalStats.BoostUse   += Part->BoostUseBonus;
		FinalStats.BoostRegen += Part->BoostRegenBonus;

	}



	//-------------------------------------------------------
	// 실제 캐릭터에 반영
	//-------------------------------------------------------
	if (auto Move = OwnerChar->GetCharacterMovement())
	{
		Move->MaxWalkSpeed = FinalStats.MoveSpeed;
		OwnerChar->WalkSpeed = FinalStats.MoveSpeed;
	}

	if (OwnerChar->HealthComp)
		OwnerChar->HealthComp->MaxHealth = FinalStats.MaxHealth;

	if (OwnerChar->ShieldComp)
		OwnerChar->ShieldComp->MaxShield = FinalStats.MaxShield;
	if (OwnerChar->Fuel)
	{
		OwnerChar->Fuel->MaxFuel = FinalStats.MaxFuel;
		OwnerChar->Fuel->BoostCost *= FinalStats.BoostUse;
		OwnerChar->Fuel->RechargeRate = FinalStats.BoostRegen;
	}
}

void UPlayerStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
