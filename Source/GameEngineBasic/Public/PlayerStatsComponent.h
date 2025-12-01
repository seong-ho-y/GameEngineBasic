// PlayerStatsComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"

USTRUCT(BlueprintType)
struct FBaseStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxShield = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostUse = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BoostRegen = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 0.f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEENGINEBASIC_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerStatsComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	FBaseStats BaseStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Stats")
	FBaseStats FinalStats;

	// 나중에 파츠 DataTable 연결할 용도 (없으면 그냥 null 두면 됨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stats")
	UDataTable* PartTable;

public:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// PlayerState → Inventory → 파츠 → 스탯 적용
	void ApplyParts();
};
