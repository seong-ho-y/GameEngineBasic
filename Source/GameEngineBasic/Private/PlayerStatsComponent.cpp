// PlayerStatsComponent.cpp

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

	// BeginPlay에서 바로 ApplyParts를 호출하지 않음.
	// 실제 적용은 캐릭터 BeginPlay에서 수행하도록 설계.
}

/*
 ============================================================
  ApplyParts()
  인벤토리 장착된 3개 파츠(Core, Upper, Lower)를 전부 읽고
  FinalStats를 갱신 → 캐릭터 능력치에 반영
 ============================================================
*/
void UPlayerStatsComponent::ApplyParts()
{
	//-------------------------------------------------------
	// 0) Owner 캐릭터 찾기
	//-------------------------------------------------------
	ASpaceCharacter* OwnerChar = Cast<ASpaceCharacter>(GetOwner());
	if (!OwnerChar)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerStatsComponent: Owner is not ASpaceCharacter"));
		return;
	}

	//-------------------------------------------------------
	// 1) PlayerState → InventoryComponent 찾기
	//-------------------------------------------------------
	AMyPlayerState* PS = OwnerChar->GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->Inventory)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerStatsComponent: Inventory not found"));
		return;
	}

	UInventoryComponent* Inv = PS->Inventory;
	if (!Inv->PartTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerStatsComponent: PartTable is NULL"));
	}

	//-------------------------------------------------------
	// 2) FinalStats 초기화 (기본 스탯 복사)
	//-------------------------------------------------------
	FinalStats = BaseStats;

	//-------------------------------------------------------
	// 3) 파츠 3종(Core / Upper / Lower) 적용
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
		if (!Part) continue;

		FinalStats.MaxHealth    += Part->HealthBonus;
		FinalStats.MaxShield    += Part->ShieldBonus;
		FinalStats.MoveSpeed    += Part->MoveSpeedBonus;
		FinalStats.BoostUse     += Part->BoostUseBonus;
		FinalStats.BoostRegen   += Part->BoostRegenBonus;
		FinalStats.Weight       += Part->WeightBonus;
	}

	//-------------------------------------------------------
	// 4) 이동속도 반영
	//-------------------------------------------------------
	if (auto Move = OwnerChar->GetCharacterMovement())
	{
		Move->MaxWalkSpeed = FinalStats.MoveSpeed;
	}

	//-------------------------------------------------------
	// 5) 체력 / 쉴드 반영
	//-------------------------------------------------------
	if (OwnerChar->HealthComp)
	{
		OwnerChar->HealthComp->MaxHealth = FinalStats.MaxHealth;

		// 필요 시 현재 체력을 MaxHealth로 맞추고 싶다면 ↓
		// OwnerChar->HealthComp->CurrentHealth = FinalStats.MaxHealth;
	}

	if (OwnerChar->ShieldComp)
	{
		OwnerChar->ShieldComp->MaxShield = FinalStats.MaxShield;

		// 필요 시 현재쉴드도 동일 처리
		// OwnerChar->ShieldComp->CurrentShield = FinalStats.MaxShield;
	}

	//-------------------------------------------------------
	// 6) Boost 관련 스탯 적용 (추후 시스템 연동 예정)
	//-------------------------------------------------------
	// OwnerChar->FuelComp 같은 것이 있다면 여기에 적용
	// 예:
	// if (OwnerChar->FuelComp)
	// {
	//     OwnerChar->FuelComp->BoostCost = FinalStats.BoostUse;
	//     OwnerChar->FuelComp->BoostRegenRate = FinalStats.BoostRegen;
	// }

	UE_LOG(LogTemp, Log, TEXT("Stats Applied: HP=%.1f SH=%.1f MOV=%.1f"),
		FinalStats.MaxHealth, FinalStats.MaxShield, FinalStats.MoveSpeed);
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
			/* Key */ 9991,
			/* Time */ 0.f,  // 0 = 매 프레임 갱신
			/* Color */ FColor::Cyan,
			DebugMsg
		);
	}
#endif
}