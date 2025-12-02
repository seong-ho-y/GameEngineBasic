// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/U_Interactable.h"
#include "Components/TimelineComponent.h"
#include "NiagaraSystem.h"

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
	Boost	  UMETA(DisplayName = "Boost"),
};

class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class USceneComponent;
class UNiagaraComponent;

UCLASS()
class GAMEENGINEBASIC_API AAbilityUnlockItem : public AActor, public IU_Interactable
{
	GENERATED_BODY()
	
public:
	AAbilityUnlockItem();

protected:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* CollisionSphere;

	// ---------- Box ----------
	UPROPERTY(VisibleAnywhere, Category = "Box")
	UStaticMeshComponent* BoxBody;

	UPROPERTY(VisibleAnywhere, Category = "Box")
	UStaticMeshComponent* BoxLid;

	UPROPERTY(EditAnywhere, Category = "Box")
	UCurveFloat* LidOpenCurve;

	// ---------- Ability ----------
	UPROPERTY(EditAnywhere, Category = "Ability")
	EAbilityType AbilityToUnlock = EAbilityType::Sprint;

	// ---------- UI ----------
	UPROPERTY(VisibleAnywhere, Category = "UI")
	class UWidgetComponent* InteractWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> AbilityUI;

	// ---------- Niagara ----------
	UPROPERTY(VisibleAnywhere, Category = "FX")
	UNiagaraComponent* Effect;


protected:
	virtual void BeginPlay() override; 
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep,
		const FHitResult& SweepResult
	);

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 BodyIndex
	);

	// ---------- Timeline Functions ----------
	UFUNCTION()
	void HandleLidOpenProgress(float Value);

public:
	virtual void Interact(ASpaceCharacter* Character) override;

	FTimeline LidOpenTimeline;
};
