// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/System/LevelSoundManager.h"

#include "Components/AudioComponent.h"

#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

ALevelSoundManager::ALevelSoundManager()
{
	PrimaryActorTick.bCanEverTick = true;

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	RootComponent = AudioComponent;
}

void ALevelSoundManager::BeginPlay()
{
	Super::BeginPlay();
	
	FString CurrentLevelName = GetWorld()->GetMapName();
	EGameLevel Level = EGameLevel::LV_TestJMars;  // 기본 값 설정

	// 레벨 이름에 맞는 Enum을 설정
	if (CurrentLevelName.Contains("TestJBoss"))
	{
		Level = EGameLevel::LV_TestJBoss;
	}
	else if (CurrentLevelName.Contains("TestJMars"))
	{
		Level = EGameLevel::LV_TestJMars;
	}
	else if (CurrentLevelName.Contains("TestJJupiter"))
	{
		Level = EGameLevel::LV_TestJJupiter;
	}
	else if (CurrentLevelName.Contains("TestMainmenu"))
	{
		Level = EGameLevel::LV_TestMainmenu;
	}

	// Play SoundCue
	PlayLevelSoundCue(Level);
}

void ALevelSoundManager::PlayLevelSoundCue(EGameLevel Level)
{
	if (LevelSoundCues.Contains(Level))
	{
		USoundCue* SelectedSoundCue = LevelSoundCues[Level];

		// If a sound cue is found, set it and play it
		if (SelectedSoundCue)
		{
			AudioComponent->SetSound(SelectedSoundCue);
			AudioComponent->Play();

			// Bind the audio finished event without parameters
			AudioComponent->OnAudioFinished.AddDynamic(this, &ALevelSoundManager::PlayNextSoundCue);
		}
	}
}

void ALevelSoundManager::PlayNextSoundCue()
{
	FString CurrentLevelName = GetWorld()->GetMapName();
	EGameLevel Level = EGameLevel::LV_TestJMars;  

	if (CurrentLevelName.Contains("TestJBoss"))
	{
		Level = EGameLevel::LV_TestJBoss;
	}
	else if (CurrentLevelName.Contains("TestJMars"))
	{
		Level = EGameLevel::LV_TestJMars;
	}
	else if (CurrentLevelName.Contains("TestJJupiter"))
	{
		Level = EGameLevel::LV_TestJJupiter;
	}
	else if (CurrentLevelName.Contains("TestMainmenu"))
	{
		Level = EGameLevel::LV_TestMainmenu;
	}

	PlayLevelSoundCue(Level);
}
