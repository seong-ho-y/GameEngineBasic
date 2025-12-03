// MyPlayerState.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

class UInventoryComponent;

USTRUCT(BlueprintType)
struct FUnlockStatus
{
	GENERATED_BODY()

	UPROPERTY()
	TSet<FName> UnlockedWeapons;

	UPROPERTY()
	TSet<FName> UnlockedParts;
};

USTRUCT(BlueprintType)
struct FAbilityUnlockStatus
{
	GENERATED_BODY()

	UPROPERTY() bool bCanSprint = false;
	UPROPERTY() bool bCanDash = false;
	UPROPERTY() bool bCanFly = false;
	UPROPERTY() bool bCanShield = false;
	UPROPERTY() bool bCanBoost = false;
};

UCLASS()
class GAMEENGINEBASIC_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMyPlayerState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unlock")
	FUnlockStatus UnlockStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FAbilityUnlockStatus AbilityStatus;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UInventoryComponent* Inventory;

protected:
	virtual void BeginPlay() override;

public:
	// 기능: 무기 / 파츠 언락
	UFUNCTION()
	void UnlockWeapon(FName WeaponRowName);

	UFUNCTION()
	void UnlockPart(FName PartRowName);

	// 기능: 이미 해금했는지 검사
	UFUNCTION(BlueprintCallable)
	bool IsWeaponUnlocked(FName WeaponRowName) const;

	UFUNCTION(BlueprintCallable)
	bool IsPartUnlocked(FName PartRowName) const;

	// 기능: 능력 해금
	UFUNCTION()
	void UnlockAbility(EAbilityType Ability);
	
	// 기능: 능력 사용 가능 여부 검사
	UFUNCTION(BlueprintCallable)
	bool CanUseAbility(EAbilityType Ability) const;

};
