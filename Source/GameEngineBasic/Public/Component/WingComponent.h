// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WingComponent.generated.h"

USTRUCT(BlueprintType)
struct FWingEffectPreset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UParticleSystem* Effect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> SocketNames;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UWingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UWingComponent();

protected:
    virtual void BeginPlay() override;

public:
    // 상태별 프리셋
    UPROPERTY(EditAnywhere, Category = "WingEffect")
    FWingEffectPreset SprintPreset;   

    UPROPERTY(EditAnywhere, Category = "WingEffect")
    FWingEffectPreset JumpPreset;    

    UPROPERTY(EditAnywhere, Category = "WingEffect")
    FWingEffectPreset FlyPreset;     

private:
    UPROPERTY()
    TArray<UParticleSystemComponent*> ActiveWingEffects;

    UPROPERTY()
    UParticleSystem* CurrentTemplate = nullptr;

    USkeletalMeshComponent* MeshComp = nullptr;

public:
    void PlaySprint();
    void PlayJump();
    void PlayFly();
    void StopAll();
    void SetMesh(USkeletalMeshComponent* NewMesh);
private:
    void SpawnPreset(const FWingEffectPreset& Preset);
    void StopActive();
};
