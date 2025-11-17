// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/public/Component/WingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"


UWingComponent::UWingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWingComponent::BeginPlay()
{

    if (AActor* Owner = GetOwner())
    {
        MeshComp = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
}

void UWingComponent::SetMesh(USkeletalMeshComponent* NewMesh)
{
    MeshComp = NewMesh;
}

void UWingComponent::SpawnPreset(const FWingEffectPreset& Preset)
{
    if (!MeshComp || !Preset.Effect)
        return;

    if (CurrentTemplate == Preset.Effect)
        return;

    StopActive();
    CurrentTemplate = Preset.Effect;

    for (const FName& Socket : Preset.SocketNames)
    {
        UParticleSystemComponent* Comp =
            UGameplayStatics::SpawnEmitterAttached(
                Preset.Effect,
                MeshComp,
                Socket,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::SnapToTarget,
                true
            );
        Comp->SetWorldScale3D(FVector(0.7f));
        ActiveWingEffects.Add(Comp);
    }
}

void UWingComponent::StopActive()
{
    for (auto* Comp : ActiveWingEffects)
    {
        if (Comp)
            Comp->DeactivateSystem();
    }
    ActiveWingEffects.Empty();
    CurrentTemplate = nullptr;
}

void UWingComponent::PlaySprint()
{
    SpawnPreset(SprintPreset);
}

void UWingComponent::PlayJump()
{
    SpawnPreset(JumpPreset);
}

void UWingComponent::PlayFly()
{
    SpawnPreset(FlyPreset);
}

void UWingComponent::StopAll()
{
    StopActive();
}

