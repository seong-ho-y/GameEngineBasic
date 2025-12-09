// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTurret.h"

#include "AIController.h"
#include "TurretAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyTurret::AEnemyTurret()
{
	PrimaryActorTick.bCanEverTick = true;
	
	GetCharacterMovement()->bOrientRotationToMovement = false;
	TurretLaserComp = CreateDefaultSubobject<UTurretLaserComponent>(TEXT("TurretLaserComp"));
}

void AEnemyTurret::BeginPlay()
{
	Super::BeginPlay();
	TurretAnim = Cast<UTurretAnimInstance>(GetMesh()->GetAnimInstance());
}

void AEnemyTurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTurretDead && !bTurretStunned) UpdateTurretHead(DeltaTime);
	if (TurretLaserComp)
	{
		AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		TurretLaserComp->SetCurrentTarget(Player);
	}
}

void AEnemyTurret::UpdateTurretHead(float DeltaSeconds)
{
	if (!TurretAnim)
		return;

	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player)
		return;

	// 본 위치 기준
	FVector BoneLocation = GetMesh()->GetBoneLocation(HeadBoneName);
	FVector TargetLocation = Player->GetActorLocation();

	FVector Dir = TargetLocation - BoneLocation;
	

	FRotator TargetRot = Dir.Rotation();

	// 부드럽게 회전(보간)
	FRotator SmoothRot = FMath::RInterpTo(
		TurretAnim->HeadAimRotation,
		TargetRot,
		DeltaSeconds,
		RotateSpeed
	);


	TurretAnim->HeadAimRotation = SmoothRot;
}
void AEnemyTurret::OnKnock()
{
	if (bTurretDead) return;

	bTurretStunned = true;

	// 레이저 끄기
	if (TurretLaserComp)
	{
		TurretLaserComp->SetLaserEnabled(false);
		TurretLaserComp->OnStunStateChanged(true);
	}
	
	

	// 0.3초 후 회전 및 레이저 재활성
	GetWorldTimerManager().SetTimer(TimerHandle_Stun, this, 
		&AEnemyTurret::EndStun, 0.35f, false);
}
void AEnemyTurret::EndStun()
{
	if (bTurretDead) return;

	bTurretStunned = false;

	if (TurretLaserComp)
	{
		TurretLaserComp->OnStunStateChanged(false);
		TurretLaserComp->SetLaserEnabled(true);
	}
}
void AEnemyTurret::OnDie(AActor* DeadActor)
{
	if (bTurretDead) return;
	bTurretDead = true;

	// 레이저 완전 끄기
	if (TurretLaserComp)
	{
		TurretLaserComp->SetLaserEnabled(false);
		TurretLaserComp->OnStunStateChanged(true);
		TurretLaserComp->SetComponentTickEnabled(false);
	}

	// 회전 정지
	bTurretStunned = true;

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 캡슐 충돌 끄기

	// AI 로직 정지
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
			Brain->StopLogic(TEXT("Turret Dead"));
	}
	

	SetLifeSpan(3.f);
}
