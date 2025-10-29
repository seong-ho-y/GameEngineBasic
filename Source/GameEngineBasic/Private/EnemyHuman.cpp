// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHuman.h"

#include "Engine/DamageEvents.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "Perception/AIPerceptionComponent.h"

// Sets default values
AEnemyHuman::AEnemyHuman()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	ShooterComp = CreateDefaultSubobject<UShooterComp>(TEXT("ShooterComp"));
	HealthComp = CreateDefaultSubobject<UHealthComp>(TEXT("HealthComp"));

	PawnSensingComp->bOnlySensePlayers = true;
	PawnSensingComp->SensingInterval = 0.1f;	
}

// Called when the game starts or when spawned
void AEnemyHuman::BeginPlay()
{
	Super::BeginPlay();
	
}



float AEnemyHuman::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                              class AController* EventInstigator, AActor* DamageCauser)
{
	const FPointDamageEvent* PointDamage = static_cast<const FPointDamageEvent*>(&DamageEvent);
	if (!PointDamage) return 0.f;

	const FHitResult& HitInfo = PointDamage->HitInfo;
	FName Bone = HitInfo.BoneName;

	float FinalDamage = DamageAmount;
	if (Bone == "spine_05" || Bone == "lowerarm_r") //몸체, 총이여서 데미지 반감
	{
		FinalDamage *= 0.75f;
	}
	else if (Bone == "pelvis" || Bone == "spine_02") //급소 데미지 보정
	{
		FinalDamage *= 1.5f;
	}

	// Groggy 시스템
	BodyPartDamage[Bone] += FinalDamage;
	if (BodyPartDamage[Bone] > GroggyThreshold[Bone])
	{
		EntryGroggyState(Bone);
	}

	CurrentHealth -= FinalDamage;
	if (CurrentHealth<=0) OnDie();
	return FinalDamage;
}
void AEnemyHuman::EntryGroggyState(FName Bone)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy entered to GroggyState"));
	//애니메이션 로직 및 움직임 로직 등등
	//Broadcast로 하는게 좋을듯 <- 맞나?
}

void AEnemyHuman::OnDie()
{
}

// Called every frame
void AEnemyHuman::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyHuman::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

