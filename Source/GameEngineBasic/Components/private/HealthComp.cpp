// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/Components/public/HealthComp.h"

// Sets default values for this component's properties
UHealthComp::UHealthComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UHealthComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHealthComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComp::InitStats()
{
}

void UHealthComp::TakeDamage()
{
}

void UHealthComp::Heal()
{
}

/*
 *만들거 : 쉴드 회복 로직
 *쉴드 있는지 확인하는거 (bool) -> TakeDamage에서 써야됨
 *VFX
*/