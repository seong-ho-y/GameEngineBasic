// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyHuman.h"

#include "AIController.h"
#include "EnemyAnimInstance.h"
#include "EnemyShieldComponent.h"
#include "Component/ExecutionComp.h"
#include "Components/CapsuleComponent.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
AEnemyHuman::AEnemyHuman()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	ShooterComp = CreateDefaultSubobject<UShooterComp>(TEXT("ShooterComp"));
	HealthComp = CreateDefaultSubobject<UHealthComp>(TEXT("HealthComp"));
	ShieldComp = CreateDefaultSubobject<UEnemyShieldComponent>(TEXT("ShieldComp"));
	
	PawnSensingComp->bOnlySensePlayers = true;
	PawnSensingComp->SensingInterval = 0.1f;
	
	ShieldComp->OnShieldBreak.AddDynamic(this, &AEnemyHuman::OnKnock);
	HealthComp->OnDeath.AddDynamic(this, &AEnemyHuman::OnDie);
	//ShieldComp->OnShieldRestored.AddDynamic(this, &AEnemyHuman::OnExecuteTimeFinish);
	
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
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn) return;
	if (auto* ExecComp = PlayerPawn->FindComponentByClass<UExecutionComp>())
	{
		ExecComp->OnExecutionStart.AddDynamic(this, &AEnemyHuman::OnExecutionStart);
		//ExecComp->OnExecutionEnd.AddDynamic(this, &AEnemyHuman::OnExecutionEnd);
	}
	USkeletalMeshComponent* LocalMesh = GetMesh();
	if (!LocalMesh) return;

	DynamicMIDs.Empty();

	for (int32 i = 0; i < LocalMesh->GetNumMaterials(); i++)
	{
		UMaterialInstanceDynamic* MID = LocalMesh->CreateAndSetMaterialInstanceDynamic(i);
		if (MID)
		{
			DynamicMIDs.Add(MID);
		}
	}
	
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

void AEnemyHuman::SetOutlineEnabled(bool bCond)
{
	bPulseActive = bCond;

	for (UMaterialInstanceDynamic* MID : DynamicMIDs)
	{
		if (!MID) continue;

		MID->SetVectorParameterValue("Base_Color", bCond ?
			FLinearColor(1, 0.1, 0.1) :
			FLinearColor(1, 1, 1));

		MID->SetScalarParameterValue("Emissive_power", bCond ? 50.f : 1.f);
	}

	if (!bCond)
	{
		PulseTime = 0.f;
		// Contrast 원상복구
		for (UMaterialInstanceDynamic* MID : DynamicMIDs)
		{
			MID->SetScalarParameterValue("Base_Constrast", 1.f);
		}
	}
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
	if (bIsExecuting) return;
	if (HealthComp->CurrentHealth <= 0) return;
	if (ShieldComp->CanBeExecuted())
		SetOutlineEnabled(true);
	else
		SetOutlineEnabled(false);
	
	USkeletalMeshComponent* LocalMesh = GetMesh();
	if (bPulseActive)
	{
		PulseTime += DeltaTime;
		// 0 ~ 5 사이 펄스
		float PulseValue = 2.5f + FMath::Sin(PulseTime * 5.0f) * 2.5f;

		if (!LocalMesh) return;

		for (int32 i = 0; i < LocalMesh->GetNumMaterials(); i++)
		{
			if (UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(LocalMesh->GetMaterial(i)))
			{
				MID->SetScalarParameterValue("Base_Constrast", PulseValue);
			}
		}
	}
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
	bIsKnocked = true;
	if (bIsBoosting)
	{
		EndBoost(); // Boost 중이면 강제 종료
	}
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->Montage_Play(KnockMontage);
	}
}

void AEnemyHuman::OnDie(AActor* DeadActor)
{
	GEngine->AddOnScreenDebugMessage(234, 1.f, FColor::Orange, TEXT("Enemy Die"));
	// ---- 중복 방지 ----
	if (bIsDead) return;
	bIsDead = true;

	// 실행중/그로기 상태 등 초기화
	bIsExecuting = false;
	bIsKnocked = false;

	// ---- Movement / AI 완전 정지 ----
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
	}

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Enemy Died"));
		}
		AICon->StopMovement();
	}

	// ---- Animation ----
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		// ★★ Blend out 없이 완전히 재생되도록 설정 ★★
		Anim->Montage_Play(DeathMontage, 1.f);
		
	}

	// ---- Collision 제거 ----
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ---- Death 처리 ----
	SetLifeSpan(8.f);
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


void AEnemyHuman::OnExecutionStart(AActor* TargetEnemy)
{
	if (TargetEnemy != this) return;
	
	SetOutlineEnabled(false);
	bIsExecuting = true;
	bIsKnocked = false;
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
		Anim->Montage_Play(ExecutionMontage);

	DisabledMovementAndAI();
}
void AEnemyHuman::DisabledMovementAndAI()
{
	// 1) 이동 중지 및 Movement 비활성화
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		// 즉시 속도 0
		Move->StopMovementImmediately();

		// 이동 자체를 비활성화
		Move->DisableMovement();

		// AI 이동이나 Force 등도 비활성화
		Move->SetComponentTickEnabled(false);
	}

	// 2) AIController + BehaviorTree 중지
	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		// AI가 가지고 있는 행동 중단
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Execution Kill Stop"));
		}

		// 이동 중지
		AICon->StopMovement();

		// AI Tick도 꺼버릴 수 있음(선택)
		AICon->SetActorTickEnabled(false);
	}

	// 3) Enemy 자체 Tick 꺼도 됨(선택)
	//SetActorTickEnabled(false);

	UE_LOG(LogTemp, Warning, TEXT("Enemy Movement & AI Disabled"));
}
