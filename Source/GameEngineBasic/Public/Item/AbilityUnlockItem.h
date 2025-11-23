// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilityUnlockItem.generated.h"

// ---------------------------
// 능력 타입 ENUM (통합)
// ---------------------------
UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	Sprint     UMETA(DisplayName = "Sprint"),
	Flying     UMETA(DisplayName = "Flying"),
	Dash       UMETA(DisplayName = "Dash"),
	Shield     UMETA(DisplayName = "Shield"),
};

class USphereComponent;
class UStaticMeshComponent;

UCLASS()
class GAMEENGINEBASIC_API AAbilityUnlockItem : public AActor
{
	GENERATED_BODY()
	
public:
	AAbilityUnlockItem();

protected:
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(EditAnywhere, Category = "Ability")
	EAbilityType AbilityToUnlock = EAbilityType::Sprint;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep,
		const FHitResult& SweepResult
	);
};
