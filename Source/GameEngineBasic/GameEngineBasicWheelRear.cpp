// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameEngineBasicWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UGameEngineBasicWheelRear::UGameEngineBasicWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}