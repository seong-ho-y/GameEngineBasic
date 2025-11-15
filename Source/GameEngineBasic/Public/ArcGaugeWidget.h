// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ArcGaugeWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class GAMEENGINEBASIC_API UArcGaugeWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetPercent(float InPercent);

	/** ArcRange(0~1) 설정하고 싶으면 추가 옵션 */
	//UFUNCTION(BlueprintCallable)
	//void SetArcRange(float InRange);

protected:

	UPROPERTY(meta = (BindWidget))
	UImage* GaugeImage;   // 여기에 UMG의 이미지 하나 바인딩

	UPROPERTY()
	UMaterialInstanceDynamic* DynMat;

	UPROPERTY(EditAnywhere, Category="ArcGauge")
	FName PercentParamName = TEXT("Percent");

	UPROPERTY(EditAnywhere, Category="ArcGauge")
	FName ArcRangeParamName = TEXT("ArcRange");
};
