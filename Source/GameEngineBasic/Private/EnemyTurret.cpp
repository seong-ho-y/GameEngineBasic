// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyTurret.h"

#include "TurretAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	
	UpdateTurretHead(DeltaTime);
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