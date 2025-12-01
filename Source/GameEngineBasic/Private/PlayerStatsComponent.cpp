// PlayerStatsComponent.cpp

#include "PlayerStatsComponent.h"
#include "SpaceCharacter/SpaceCharacter.h"
#include "MyPlayerState.h"
#include "InventoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UPlayerStatsComponent::UPlayerStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	// 캐릭터 BeginPlay에서 다시 ApplyParts 호출하니까
	// 여기서는 굳이 안 불러도 되지만, 안전하게 한 번 더
	// ApplyParts();
}

void UPlayerStatsComponent::ApplyParts()
{
	ASpaceCharacter* OwnerChar = Cast<ASpaceCharacter>(GetOwner());
	if (!OwnerChar) return;

	AMyPlayerState* PS = OwnerChar->GetPlayerState<AMyPlayerState>();
	if (!PS || !PS->Inventory)
		return;

	// 1) 기본 스탯으로 초기화
	FinalStats = BaseStats;

	// 2) TODO: PartTable + PS->Inventory->GetPart(EPartSlot::Core/Upper/Lower)
	//    로 파츠 스탯을 더하는 로직을 나중에 추가하면 됨.
	//    지금은 구조만 만들어두고 기본값만 사용.

	// 3) 캐릭터에 실제 적용 (일단 이동속도만 확실하게 반영)
	if (UCharacterMovementComponent* MoveComp = OwnerChar->GetCharacterMovement())
	{
		MoveComp->MaxWalkSpeed = FinalStats.MoveSpeed;
	}

	// 체력/쉴드/부스트 등은 기존 HealthComp / ShieldComp / FuelComp API 보고
	// 나중에 여기서 같이 묶어주면 됨.
}
