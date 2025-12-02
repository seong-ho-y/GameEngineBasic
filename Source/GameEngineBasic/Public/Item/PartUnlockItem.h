// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"


#include "Components/TimelineComponent.h"
#include "InventoryComponent.h"

#include "NiagaraSystem.h"
#include "Item/U_Interactable.h"

#include "PartUnlockItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class USceneComponent;
class UNiagaraComponent;

UCLASS()
class GAMEENGINEBASIC_API APartUnlockItem : public AActor, public IU_Interactable
{
	GENERATED_BODY()
	
public:
	APartUnlockItem();

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

	// ---------- UI ----------
	UPROPERTY(VisibleAnywhere, Category = "UI")
	class UWidgetComponent* InteractWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> AbilityUI;

	// ---------- Niagara ----------
	UPROPERTY(VisibleAnywhere, Category = "FX")
	UNiagaraComponent* Effect;

public:
	/** Core / Upper / Lower */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	EPartSlot Slot;

	/** Part RowName */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part")
	FName PartRowName;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
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
