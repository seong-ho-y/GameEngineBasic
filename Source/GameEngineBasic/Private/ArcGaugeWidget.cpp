// Fill out your copyright notice in the Description page of Project Settings.


#include "ArcGaugeWidget.h"

#include "Components/Image.h"

void UArcGaugeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GaugeImage)
		DynMat = GaugeImage->GetDynamicMaterial();
}

void UArcGaugeWidget::SetPercent(float InPercent)
{
	if (DynMat)
		DynMat->SetScalarParameterValue(PercentParamName, InPercent);
}
