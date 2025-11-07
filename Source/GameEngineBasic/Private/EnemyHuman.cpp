// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHuman.h"

#include "EnemyAnimInstance.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/DamageEvents.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	//몸체 자체 회전 끄기
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로만 몸 돌림
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f); // 원하는 회전 속도
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

void AEnemyHuman::StartBoost(FVector Direction, float Speed, float Duration, float Decel, float GravityScale)
{
	if (bIsBoosting) return;
	bIsBoosting = true;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) { bIsBoosting = false; return; }

	// 캐시
	BoostElapsed           = 0.f;
	BoostDurationCached    = Duration;
	BoostSpeedCached       = Speed;
	GlideDecelRateCached   = Decel;
	BoostDirCached         = Direction.GetSafeNormal2D();
	OriginalGravityScale   = Move->GravityScale;

	// 물리 세팅
	Move->GravityScale = GravityScale;
	Move->Velocity     = BoostDirCached * BoostSpeedCached;

	// 애니메이션/이펙트
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->PlayBoostMontage();
	}
	/* Niagara 에셋 괜찮은거 없어서 보류
	if (BoostVfx)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			BoostVfx, GetMesh(), "BoostSocket",
			FVector::ZeroVector, GetActorRotation(),
			EAttachLocation::SnapToTarget, true);
	}
	*/
	if (BoostPS)
	{
		ActiveBoostPSC = UGameplayStatics::SpawnEmitterAttached(BoostPS,
			GetMesh(),
			FName("BoostSocket"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true);
	}

	// 타이머 시작 (지속 갱신)
	GetWorldTimerManager().SetTimer(
		TimerHandle_BoostTick, this, &AEnemyHuman::OnBoostTick, BoostTickInterval, true);
}

void AEnemyHuman::EndBoost()
{
	if (!bIsBoosting) return;
	bIsBoosting = false;

	// 타이머 해제
	GetWorldTimerManager().ClearTimer(TimerHandle_BoostTick);

	// 물리 원복
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->GravityScale = OriginalGravityScale;
	}

	if (ActiveBoostPSC)
	{
		ActiveBoostPSC->Deactivate();
		ActiveBoostPSC->DestroyComponent();
		 ActiveBoostPSC = nullptr;
	}
	// 애니메이션 복귀(상태 표현)
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->AnimState = EEnemyAnimState::Idle;
	}
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

void AEnemyHuman::OnBoostTick()
{
	UWorld* World = GetWorld();
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!World || !Move)
	{
		EndBoost();
		return;
	}

	const float DeltaSeconds = World->GetDeltaSeconds();
	BoostElapsed += DeltaSeconds;

	if (BoostElapsed < BoostDurationCached)
	{
		// ⛽ 부스트 유지 구간: 계속 같은 속도로 민다
		Move->Velocity = BoostDirCached * BoostSpeedCached;
		return;
	}

	// 🪂 글라이드/감속 구간: 속도를 0으로 서서히 보간
	Move->Velocity = FMath::VInterpTo(Move->Velocity, FVector::ZeroVector, DeltaSeconds, GlideDecelRateCached);

	// 종료 조건
	if (Move->Velocity.SizeSquared2D() < 10.f)
	{
		EndBoost();
	}
}

