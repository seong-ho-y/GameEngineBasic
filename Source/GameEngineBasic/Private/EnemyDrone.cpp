#include "EnemyDrone.h"
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
