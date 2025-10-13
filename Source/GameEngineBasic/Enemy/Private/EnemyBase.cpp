// Fill out your copyright notice in the Description page of Project Settings.


#include "GameEngineBasic/Enemy/Public/EnemyBase.h"

#include "AIController.h"
#include "BrainComponent.h"
#include "GameEngineBasic/Components/public/Attack.h"
#include "GameEngineBasic/Components/public/Damageable.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameFramework/CharacterMovementComponent.h"

class AAIController;
// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HealthComp = CreateDefaultSubobject<UHealthComp>("HealthComp");
	ShooterComp = CreateDefaultSubobject<UShooterComp>("ShooterComp");
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyBase::ApplyDamage_Implementation(float DamageAmount, AController* InstigatorController, AActor* DamageCauser,
	FVector HitLoc, TSubclassOf<UDamageType> DamageType)
{
	IDamageable::ApplyDamage_Implementation(DamageAmount, InstigatorController, DamageCauser, HitLoc, DamageType);
}

void AEnemyBase::Die_Implementation(AActor* Killer)
{
	IDamageable::Die_Implementation(Killer);
}

bool AEnemyBase::IsDead_Implementation() const
{
	return IDamageable::IsDead_Implementation();
}

void AEnemyBase::Attack_Implementation()
{
	IAttack::Attack_Implementation();
}

void AEnemyBase::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("%s Activated"), *GetName());

	// 1. 액터를 보이게 하고, 충돌 및 틱을 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	// 2. 컴포넌트의 상태를 초기화합니다.
	// HealthComp에 체력과 쉴드를 최대로 설정해주기
	// Descriptor를 써서 할거같긴함
	if (HealthComp)
	{
		HealthComp->InitStats();
	}

	// ShooterComp 등 다른 컴포넌트
	// if (ShooterComp)
	// {
	//     ShooterComp->ResetAmmo(); 
	// }

	// 3. AI 로직을 다시 시작
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBrainComponent())
	{
		AIController->GetBrainComponent()->StartLogic();
	}
    
	// 4. 캐릭터 이동을 활성화
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->Activate();
	}
}
void AEnemyBase::DeActivate()
{
	UE_LOG(LogTemp, Warning, TEXT("%s Deactivated"), *GetName());

	// 1. 액터를 숨기고, 충돌 및 틱을 비활성화하여 성능 부하를 줄입니다.
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// 2. AI 로직을 정지시킵니다.
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBrainComponent())
	{
		AIController->GetBrainComponent()->StopLogic("Deactivated by Object Pool");
	}

	// 3. 캐릭터의 움직임을 즉시 멈추고 비활성화합니다.
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
}