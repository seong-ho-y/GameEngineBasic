#include "EnemyDrone.h"

#include "AIController.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyDrone::AEnemyDrone()
{
	PrimaryActorTick.bCanEverTick = true;
	// 공중 이동 전용 모드
	DroneMoveComp = CreateDefaultSubobject<UDroneMovementComponent>(TEXT("DroneMoveComp"));
	// CharacterMovement 제거
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DestroyComponent();
	}
	bUseControllerRotationYaw = false;


}

void AEnemyDrone::BeginPlay()
{
	Super::BeginPlay();
}

void AEnemyDrone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	TurnToFacePlayer(DeltaTime);
}


/* ==========================================================
   Player 바라보기 (Yaw only)
   ========================================================== */
void AEnemyDrone::TurnToFacePlayer(float DeltaSeconds)
{
	AActor* Player = GetPlayerPawn();
	if (!Player) return;

	FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	ToPlayer.Z = 0.f;
	FRotator TargetRot = ToPlayer.Rotation();

	FRotator NewRot = FMath::RInterpTo(
		GetActorRotation(),
		TargetRot,
		DeltaSeconds,
		RotationInterpSpeed);

	SetActorRotation(NewRot);
}


/* ==========================================================
   Dodge (좌/우 측면 회피)
   ========================================================== */
void AEnemyDrone::StartDodge()
{
	if (bDodging) return;

	bDodging = true;

	// 좌 / 우 랜덤
	float LR = FMath::RandBool() ? 1.f : -1.f;
	DodgeDirection = GetActorRightVector() * LR;

	GetCharacterMovement()->Velocity = DodgeDirection * DodgeSpeed;

	GetWorldTimerManager().SetTimer(
		TimerHandle_Dodge,
		this,
		&AEnemyDrone::EndDodge,
		DodgeTime,
		false
	);
}

void AEnemyDrone::EndDodge()
{
	bDodging = false;
	GetCharacterMovement()->StopMovementImmediately();
}


/* ==========================================================
   Player Getter
   ========================================================== */
AActor* AEnemyDrone::GetPlayerPawn() const
{
	return UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
}

void AEnemyDrone::OnKnock()
{
	if (bIsDead) return;
	

	// ✔ 넉백 방향 (플레이어 반대 방향)
	AActor* Player = GetPlayerPawn();
	if (Player)
	{
		FVector Dir = (GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();
		FVector KnockVelocity = Dir * KnockbackStrength;

		// ✔ DroneMovementComponent가 직접 Velocity 세팅
		if (DroneMoveComp)
		{
			DroneMoveComp->Velocity = KnockVelocity;
		}
	}

	// ✔ 짧은 Stun 느낌을 위해 0.2초간 움직임 제한도 가능
	bDodging = true;
	GetWorldTimerManager().SetTimer(
		TimerHandle_KnockStun,
		FTimerDelegate::CreateLambda([this]()
		{
			bDodging = false;
		}),
		0.2f,
		false
	);
}


void AEnemyDrone::OnDie(AActor* DeadActor)
{
	if (bIsDead) return;
	bIsDead = true;

	// ✔ 이동 중단
	if (DroneMoveComp)
	{
		DroneMoveComp->Velocity = FVector::ZeroVector;
	}

	// ✔ AI 중지
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->StopMovement();
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
			Brain->StopLogic(TEXT("Drone Dead"));
	}

	// ✔ 충돌 끈다 (공중에서 떨어질 때 안 걸리게)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ✔ 물리 켜서 "힘 빠져 추락" 연출
	USkeletalMeshComponent* _Mesh = GetMesh();
	if (_Mesh)
	{
		_Mesh->SetSimulatePhysics(true);
		_Mesh->SetEnableGravity(true);

		// 자연스러운 회전
		_Mesh->AddAngularImpulseInDegrees(FVector(
			FMath::RandRange(-400, 400),
			FMath::RandRange(-400, 400),
			FMath::RandRange(-400, 400)
		));
	}
	
	

	SetLifeSpan(4.f);
}
