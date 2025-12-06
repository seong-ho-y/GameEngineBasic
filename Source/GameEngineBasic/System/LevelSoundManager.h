// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"

#include "Components/AudioComponent.h"

#include "LevelSoundManager.generated.h"

UENUM(BlueprintType)
enum class EGameLevel : uint8
{
	LV_TestJMars  UMETA(DisplayName = "Test JMars"),
	LV_TestJJupiter UMETA(DisplayName = "Test JJupiter"),
	LV_TestJBoss UMETA(DisplayName = "Test JBoss"),
	LV_TestMainmenu UMETA(DisplayName = "TestMainmenu")
};

UCLASS()
class GAMEENGINEBASIC_API ALevelSoundManager : public AActor
{
	GENERATED_BODY()
	
public:
	ALevelSoundManager();

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
	UAudioComponent* AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TMap<EGameLevel, USoundCue*> LevelSoundCues;

	
	void PlayLevelSoundCue(EGameLevel Level);

private:
	UFUNCTION()
	void PlayNextSoundCue();
};
