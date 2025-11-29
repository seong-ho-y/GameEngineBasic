// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathWidget.generated.h"

class UButton;

UCLASS()
class GAMEENGINEBASIC_API UDeathWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UButton* RespawnButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnRespawnClicked();

	UFUNCTION()
	void OnExitClicked();
};
