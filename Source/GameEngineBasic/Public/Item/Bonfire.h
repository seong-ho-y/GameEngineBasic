// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item/U_Interactable.h"


#include "Bonfire.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;
class USceneComponent;
class UNiagaraComponent;

class ASpaceCharacter;


UCLASS()
class GAMEENGINEBASIC_API ABonfire : public AActor, public IU_Interactable
{
	GENERATED_BODY()
	
public:
	ABonfire();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual void Interact(ASpaceCharacter* Character) override;

protected:
    // ---------- Components ----------
    UPROPERTY(VisibleAnywhere)
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Checkpoint")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, Category = "Checkpoint")
    UStaticMeshComponent* BonfireMesh;

	// ---------- Niagara ----------
    UPROPERTY(VisibleAnywhere, Category = "FX")
    UNiagaraComponent* Effect;

	// ---------- UI ----------
    UPROPERTY(VisibleAnywhere, Category = "UI")
    class UWidgetComponent* InteractWidget;

    UPROPERTY(EditAnywhere,  Category = "UI")
    TSubclassOf <UUserWidget> AbilityUI;

    // ---------- Data ----------
    UPROPERTY(EditAnywhere, Category = "Checkpoint")
    FName CheckpointID = TEXT("Checkpoint");

    UPROPERTY(VisibleAnywhere, Category = "Checkpoint")
    bool bActivated = false;

protected:
    // ---------- Functions ----------
    UFUNCTION()
    void OnOverlapBegin(
        UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 BodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(
        UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 BodyIndex);

    FTimerHandle NiagaraTimerHandle;
	void StartNiagara(ASpaceCharacter* Character);
	void StopNiagara();
};
