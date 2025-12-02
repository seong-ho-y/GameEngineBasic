#include "PlayerStatsComponent.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "MyPlayerState.h"
#include "InventoryComponent.h"

// 게임플레이 관련
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
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
				TEXT("[StatsComp] ERROR: Owner is not ASpaceCharacter"));
		return;
	}

	AMyPlayerState* PS = OwnerChar->GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->Inventory)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
				TEXT("[StatsComp] WARNING: Inventory not found"));
		return;
	}

	UInventoryComponent* Inv = PS->Inventory;
	if (!Inv->PartTable)
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow,
				TEXT("[StatsComp] WARNING: PartTable is NULL"));
	}

	//-------------------------------------------------------
	// FinalStats 기본 초기화
	//-------------------------------------------------------
	FinalStats = BaseStats;
	
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::White,
			TEXT("[ApplyParts] Reset FinalStats to BaseStats"));
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
			if (GEngine)
			{
				FString SlotStr =
					(Slot == EPartSlot::Core)  ? TEXT("Core") :
					(Slot == EPartSlot::Upper) ? TEXT("Upper") :
												 TEXT("Lower");

				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Silver,
					FString::Printf(TEXT("[ApplyParts] Slot %s: NO PART"), *SlotStr));
			}
			continue;
		}

		// Slot 이름 출력
		FString SlotStr =
			(Slot == EPartSlot::Core)  ? TEXT("Core") :
			(Slot == EPartSlot::Upper) ? TEXT("Upper") :
										 TEXT("Lower");

		// 파츠 능력치 로그
		if (GEngine)
		{
			FString BonusMsg = FString::Printf(
				TEXT("[ApplyParts] Slot=%s | HP=%.1f SH=%.1f Move=%.1f BoostUse=%.1f BoostRegen=%.1f Weight=%.1f"),
				*SlotStr,
				Part->HealthBonus,
				Part->ShieldBonus,
				Part->MoveSpeedBonus,
				Part->BoostUseBonus,
				Part->BoostRegenBonus,
				Part->WeightBonus
			);
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, BonusMsg);
		}

		// 스탯 적용
		FinalStats.MaxHealth  += Part->HealthBonus;
		FinalStats.MaxShield  += Part->ShieldBonus;
		FinalStats.MoveSpeed  += Part->MoveSpeedBonus;
		FinalStats.BoostUse   += Part->BoostUseBonus;
		FinalStats.BoostRegen += Part->BoostRegenBonus;
		FinalStats.Weight     += Part->WeightBonus;
	}

	//-------------------------------------------------------
	// 최종 스탯 로그
	//-------------------------------------------------------
	if (GEngine)
	{
		FString FinalMsg = FString::Printf(
			TEXT("[FinalStats READY] HP=%.1f  SH=%.1f  Move=%.1f  BoostUse=%.2f  BoostRegen=%.2f  Weight=%.1f"),
			FinalStats.MaxHealth,
			FinalStats.MaxShield,
			FinalStats.MoveSpeed,
			FinalStats.BoostUse,
			FinalStats.BoostRegen,
			FinalStats.Weight
		);

		GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, FinalMsg);
	}

	//-------------------------------------------------------
	// 실제 캐릭터에 반영
	//-------------------------------------------------------
	if (auto Move = OwnerChar->GetCharacterMovement())
	{
		Move->MaxWalkSpeed = FinalStats.MoveSpeed;
	}

	if (OwnerChar->HealthComp)
		OwnerChar->HealthComp->MaxHealth = FinalStats.MaxHealth;

	if (OwnerChar->ShieldComp)
		OwnerChar->ShieldComp->MaxShield = FinalStats.MaxShield;
}

void UPlayerStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (GEngine)
	{
		FString DebugMsg = FString::Printf(TEXT(
			"[Stats] HP=%.1f | SH=%.1f | Move=%.1f | BoostUse=%.2f | BoostRegen=%.2f | Weight=%.1f"),
			FinalStats.MaxHealth,
			FinalStats.MaxShield,
			FinalStats.MoveSpeed,
			FinalStats.BoostUse,
			FinalStats.BoostRegen,
			FinalStats.Weight
		);

		GEngine->AddOnScreenDebugMessage(
			9991,
			0.f,
			FColor::Cyan,
			DebugMsg
		);
	}
#endif
}
