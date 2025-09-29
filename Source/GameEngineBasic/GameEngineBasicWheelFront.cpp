// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameEngineBasicWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UGameEngineBasicWheelFront::UGameEngineBasicWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}