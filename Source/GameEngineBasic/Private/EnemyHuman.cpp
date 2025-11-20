// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHuman.h"

#include "EnemyAnimInstance.h"
#include "EnemyShieldComponent.h"
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
	ShieldComp = CreateDefaultSubobject<UEnemyShieldComponent>(TEXT("ShieldComp"));
	/*
	HealthComp->bUseShield = false;
	HealthComp->bUseShieldRegen = false;
	*/
	PawnSensingComp->bOnlySensePlayers = true;
	PawnSensingComp->SensingInterval = 0.1f;
	
	ShieldComp->OnShieldBreak.AddDynamic(this, &AEnemyHuman::OnKnock);
	HealthComp->OnDeath.AddDynamic(this, &AEnemyHuman::OnDie);
	
	//몸체 자체 회전 끄기
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향으로만 몸 돌림
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f); // 원하는 회전 속도

	ShooterComp->bUseAmmo = false;
}

// Called when the game starts or when spawned
void AEnemyHuman::BeginPlay()
{
	Super::BeginPlay();


	
}

float AEnemyHuman::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                              class AController* EventInstigator, AActor* DamageCauser)
{
	float FinalDamage = DamageAmount;
	if (ShieldComp)
		FinalDamage = ShieldComp->ApplyDamage(DamageAmount);

	if (HealthComp)
		HealthComp->ApplyHealthDamage(FinalDamage);
	
	return FinalDamage;
}
void AEnemyHuman::EntryGroggyState(FName Bone)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy entered to GroggyState"));
	//애니메이션 로직 및 움직임 로직 등등
	//Broadcast로 하는게 좋을듯 <- 맞나?
}

void AEnemyHuman::StartBoost(FVector Direction, float Speed, float Duration, float Decel, float GravityScale)
{
	if (bIsBoosting || GetWorldTimerManager().IsTimerActive(TimerHandle_BoostTick))
		return; // ✅ 중복 방지

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
		Anim->LowerBodyState = ELowerBodyState::Boost;
		PlayAnimMontage(BoostMontage);
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

	GetWorldTimerManager().ClearTimer(TimerHandle_BoostTick);

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

	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		SetLowerBodyState(ELowerBodyState::WalkBlendSpace);
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
		// Keep going Boost
		Move->Velocity = BoostDirCached * BoostSpeedCached;
		return;
	}

	// Interp Velocity to 0 slowly
	Move->Velocity = FMath::VInterpTo(Move->Velocity, FVector::ZeroVector, DeltaSeconds, GlideDecelRateCached);

	// Fin
	if (Move->Velocity.SizeSquared2D() < 10.f)
	{
		EndBoost();
	}
}

void AEnemyHuman::OnKnock()
{
	UE_LOG(LogTemp, Error, TEXT("Enemy Got Knocked"));
	if (bIsBoosting)
	{
		EndBoost(); // Boost 중이면 강제 종료
	}

	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->FullBodyState = EFullBodyState::Knock;
		Anim->Montage_Play(KnockMontage);
	}
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->FullBodyState = EFullBodyState::Knock;
		Anim->Montage_Play(KnockMontage);
	}
}

void AEnemyHuman::OnDie(AActor* DeadActor)
{
	static bool bDied = false;
	if (bDied) return;
	bDied = true;

	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->FullBodyState = EFullBodyState::Dead;
		Anim->Montage_Play(DeathMontage);
	}

	// Drone에서 CharacterMovement 삭제했는데 접근해서 터짐
	//GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.f);
}

void AEnemyHuman::SetLowerBodyState(ELowerBodyState NewState)
{
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
		Anim->LowerBodyState = NewState;
}

// 상체 전용 상태 변경
void AEnemyHuman::SetUpperBodyState(EUpperBodyState NewState)
{
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
		Anim->UpperBodyState = NewState;
}