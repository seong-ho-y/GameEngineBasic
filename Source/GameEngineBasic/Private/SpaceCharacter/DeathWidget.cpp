// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter/DeathWidget.h"
#include "Components/Button.h"
#include "MyPlayerController.h"


void UDeathWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (RespawnButton)
        RespawnButton->OnClicked.AddDynamic(this, &UDeathWidget::OnRespawnClicked);
    if(ExitButton)
		ExitButton->OnClicked.AddDynamic(this, &UDeathWidget::OnExitClicked);
}

void UDeathWidget::OnRespawnClicked()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Respawn Clicked"));

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC))
        {
            MyPC->RequestRespawn();
        }
    }
}

void UDeathWidget::OnExitClicked()
{
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Exit Clicked"));
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->ConsoleCommand("quit");
    }
}