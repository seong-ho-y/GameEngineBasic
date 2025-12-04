// EnemyHuman.cpp

#include "EnemyHuman.h"

#include "AIController.h"
#include "EnemyAnimInstance.h"
#include "EnemyBlade.h"
#include "EnemyShieldComponent.h"
#include "Component/ExecutionComp.h"
#include "Components/CapsuleComponent.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "BrainComponent.h"
#include "BTT_DashAttack.h"
#include "NiagaraFunctionLibrary.h"
//#include "NiagaraFunctionLibrary.h"  // BoostVfx 쓸 때 사용 가능


// Sets default values
AEnemyHuman::AEnemyHuman()
{
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	ShooterComp     = CreateDefaultSubobject<UShooterComp>(TEXT("ShooterComp"));
	HealthComp      = CreateDefaultSubobject<UHealthComp>(TEXT("HealthComp"));
	ShieldComp      = CreateDefaultSubobject<UEnemyShieldComponent>(TEXT("ShieldComp"));
	
	PawnSensingComp->bOnlySensePlayers = true;
	PawnSensingComp->SensingInterval   = 0.1f;
	
	ShieldComp->OnShieldBreak.AddDynamic(this, &AEnemyHuman::OnKnock);
	HealthComp->OnDeath.AddDynamic(this, &AEnemyHuman::OnDie);
	//ShieldComp->OnShieldRestored.AddDynamic(this, &AEnemyHuman::OnExecuteTimeFinish);
	
	// 몸체 자체 회전 끄기
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement     = true; // 이동 방향으로만 몸 돌림
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

	if (BladeBP)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;

		Blade = GetWorld()->SpawnActor<AEnemyBlade>(BladeBP, Params);

		if (Blade)
		{
			Blade->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				TEXT("MeleeSocket")
			);
			Blade->OwnerCharacter = this;
			Blade->SetActorHiddenInGame(true);
			Blade->DeactivateHitbox();
		}
	}
	if (LeftBladeBP)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;

		LeftBlade = GetWorld()->SpawnActor<AEnemyBlade>(LeftBladeBP, Params);

		if (LeftBlade)
		{
			LeftBlade->AttachToComponent(
				GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				TEXT("RedBlade")        // 왼손에 새 소켓 만들어놓기
			);

			LeftBlade->OwnerCharacter = this;
			LeftBlade->SetActorHiddenInGame(true);
			LeftBlade->DeactivateHitbox();
		}
	}

}

float AEnemyHuman::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
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

		MID->SetVectorParameterValue(
			"Base_Color",
			bCond ? FLinearColor(1, 0.1f, 0.1f) : FLinearColor(1, 1, 1)
		);

		MID->SetScalarParameterValue("Emissive_power", bCond ? 50.f : 1.f);
	}

	if (!bCond)
	{
		PulseTime = 0.f;
		for (UMaterialInstanceDynamic* MID : DynamicMIDs)
		{
			if (!MID) continue;
			MID->SetScalarParameterValue("Base_Constrast", 1.f);
		}
	}
}

void AEnemyHuman::EntryGroggyState(FName Bone)
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy entered to GroggyState"));
	// TODO: 그로기 상태 애니/로직
}

void AEnemyHuman::StartBoost(FVector Direction, float Speed, float Duration, float Decel, float GravityScale)
{
	if (bIsBoosting || GetWorldTimerManager().IsTimerActive(TimerHandle_BoostTick))
		return; // 중복 방지

	bIsBoosting = true;

	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move)
	{
		bIsBoosting = false;
		return;
	}

	BoostElapsed         = 0.f;
	BoostDurationCached  = Duration;
	BoostSpeedCached     = Speed;
	GlideDecelRateCached = Decel;
	BoostDirCached       = Direction.GetSafeNormal2D();
	OriginalGravityScale = Move->GravityScale;

	Move->GravityScale = GravityScale;
	Move->Velocity     = BoostDirCached * BoostSpeedCached;

	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->LowerBodyState = ELowerBodyState::Boost;
		PlayAnimMontage(BoostMontage);
	}

	/*
	if (BoostVfx)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			BoostVfx, GetMesh(), "BoostSocket",
			FVector::ZeroVector, GetActorRotation(),
			EAttachLocation::SnapToTarget, true);
	}
	*/
	if (BoostSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BoostSound,
			GetActorLocation()
		);
	}
	if (BoostPS)
	{
		ActiveBoostPSC = UGameplayStatics::SpawnEmitterAttached(
			BoostPS,
			GetMesh(),
			FName("BoostSocket"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	GetWorldTimerManager().SetTimer(
		TimerHandle_BoostTick,
		this,
		&AEnemyHuman::OnBoostTick,
		BoostTickInterval,
		true
	);
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
	if (HealthComp && HealthComp->CurrentHealth <= 0) return;

	// Execute 가능하면 Outline
	if (ShieldComp && ShieldComp->CanBeExecuted())
		SetOutlineEnabled(true);
	else
		SetOutlineEnabled(false);
	

	// ShortDash (Task에서 방향/속도 지정) 처리
	if (bIsShortDashing)
	{
		ShortDashElapsed += DeltaTime;
		if (ShortDashElapsed >= ShortDashDuration)
		{
			bIsShortDashing = false;

			// 감속 & 멈춤: 대쉬 후 브레이크 느낌
			if (UCharacterMovementComponent* Move = GetCharacterMovement())
			{
				Move->Velocity = FVector::ZeroVector;
			}
			if (auto* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
			{
				Anim->bIsShortDashing = false;
				Anim->LowerBodyState = ELowerBodyState::WalkBlendSpace;
			}
		}
	}

	// Melee Hitbox 처리
	if (bMeleeHitboxActive)
	{
		if (!GetMesh())
			return;

		FVector Origin = GetMesh()->GetSocketLocation(MeleeHitSocket);
		FCollisionShape Shape = FCollisionShape::MakeSphere(MeleeRange);
		

		TArray<FHitResult> Hits;
		FCollisionObjectQueryParams ObjParams;
		ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel4); // Player Object Channel

		bool bHit = GetWorld()->SweepMultiByObjectType(
			Hits,
			Origin,
			Origin,
			FQuat::Identity,
			ObjParams,
			Shape
		);

		if (bHit)
		{
			for (const FHitResult& Hit : Hits)
			{
				AActor* HitActor = Hit.GetActor();
				if (!HitActor) continue;
				if (HitActor == this) continue;

				if (MeleeAlreadyHitActors.Contains(HitActor))
					continue;

				MeleeAlreadyHitActors.Add(HitActor);

				UE_LOG(LogTemp, Warning, TEXT("EnemyHuman Melee Hit: %s"), *HitActor->GetName());

				UGameplayStatics::ApplyPointDamage(
					HitActor,
					MeleeDamage,
					GetActorForwardVector(),
					Hit,
					GetController(),
					this,
					nullptr
				);
			}
		}
	}

	// Outline Pulse
	USkeletalMeshComponent* LocalMesh = GetMesh();
	if (bPulseActive)
	{
		PulseTime += DeltaTime;
		float PulseValue = 2.5f + FMath::Sin(PulseTime * 5.0f) * 2.5f;

		if (!LocalMesh) return;

		for (int32 i = 0; i < LocalMesh->GetNumMaterials(); i++)
		{
			if (UMaterialInstanceDynamic* MID =
				Cast<UMaterialInstanceDynamic>(LocalMesh->GetMaterial(i)))
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
		Move->Velocity = BoostDirCached * BoostSpeedCached;
		return;
	}

	Move->Velocity = FMath::VInterpTo(
		Move->Velocity,
		FVector::ZeroVector,
		DeltaSeconds,
		GlideDecelRateCached
	);

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
		EndBoost();
	}

	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->Montage_Play(KnockMontage);
	}
}

void AEnemyHuman::OnDie(AActor* DeadActor)
{
	GEngine->AddOnScreenDebugMessage(234, 1.f, FColor::Orange, TEXT("Enemy Die"));

	if (bIsDead) return;
	bIsDead = true;

	bIsExecuting = false;
	bIsKnocked   = false;

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

	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->Montage_Play(DeathMontage, 1.f);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(8.f);
}

void AEnemyHuman::SetLowerBodyState(ELowerBodyState NewState)
{
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->LowerBodyState = NewState;
	}
}

void AEnemyHuman::SetUpperBodyState(EUpperBodyState NewState)
{
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->UpperBodyState = NewState;
	}
}

void AEnemyHuman::OnExecutionStart(AActor* TargetEnemy)
{
	if (TargetEnemy != this) return;
	
	SetOutlineEnabled(false);
	bIsExecuting = true;
	bIsKnocked   = false;

	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->Montage_Play(ExecutionMontage);
	}

	DisabledMovementAndAI();
}

void AEnemyHuman::DisabledMovementAndAI()
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->StopMovementImmediately();
		Move->DisableMovement();
		Move->SetComponentTickEnabled(false);
	}

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* Brain = AICon->GetBrainComponent())
		{
			Brain->StopLogic(TEXT("Execution Kill Stop"));
		}

		AICon->StopMovement();
		AICon->SetActorTickEnabled(false);
	}

	UE_LOG(LogTemp, Warning, TEXT("Enemy Movement & AI Disabled"));
}

void AEnemyHuman::StartMeleeAttack()
{
	if (!MeleeMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyHuman::StartMeleeAttack - MeleeMontage is NULL"));
		bMeleeFinished = true;
		return;
	}

	bMeleeFinished      = false;
	bMeleeHitboxActive  = false;
	MeleeAlreadyHitActors.Empty();
	
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->bIsMeleeAttacking = true;
	}
	
	PlayAnimMontage(MeleeMontage);
	UE_LOG(LogTemp, Warning, TEXT("EnemyHuman::StartMeleeAttack - Montage Started"));
}

void AEnemyHuman::OnMeleeBegin()
{
	if (Blade)
	{
		Blade->SetActorHiddenInGame(false);
		Blade->ActivateHitbox();
	}
	bMeleeHitboxActive = true;
}

void AEnemyHuman::OnMeleeEnd()
{
	if (Blade)
	{
		Blade->DeactivateHitbox();
		Blade->SetActorHiddenInGame(true);
	}
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->bIsMeleeAttacking = false;
	}
	bMeleeHitboxActive = false;
	bMeleeFinished     = true;
}

bool AEnemyHuman::IsMeleeFinished() const
{
	return bMeleeFinished;
}


void AEnemyHuman::PlayDashChargeMontage()
{
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->bIsDashAttacking = true;
		Anim->FullBodyState = EFullBodyState::DashAttack;
	}
	if (DashAttackMontage)
	{
		PlayAnimMontage(DashAttackMontage);
	}
}

void AEnemyHuman::OnLeftBladeBegin()
{
	if (LeftBlade)
	{
		LeftBlade->SetActorHiddenInGame(false);
		LeftBlade->ActivateHitbox();       // Collision On
	}
}

void AEnemyHuman::OnLeftBladeEnd()
{
	if (LeftBlade)
	{
		LeftBlade->DeactivateHitbox();     // Collision Off
		LeftBlade->SetActorHiddenInGame(true);
	}
}

void AEnemyHuman::BeginDash()
{
	AActor* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return;

	DashDir = (Player->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	

	float DashPower=  DashSpeed;
	LaunchCharacter(DashDir * DashPower, true , false);
}

void AEnemyHuman::EndDash()
{
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("EnDDash"));
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->bIsDashAttacking = false;
		Anim->FullBodyState = EFullBodyState::Default;
	}

}

/* ===========================
 *  ShortDash (방향 지정 짧은 대쉬)
 * ===========================
 */
void AEnemyHuman::StartShortDash(const FVector& Dir, float Speed, float Duration)
{
	bIsShortDashing   = true;
	ShortDashElapsed  = 0.f;
	ShortDashDuration = Duration;

	FVector DashDir_ = Dir.GetSafeNormal2D();
	// ★ 애니메이션 상태 전달
	if (UEnemyAnimInstance* Anim = Cast<UEnemyAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		Anim->bIsShortDashing = true;
		Anim->LowerBodyState = ELowerBodyState::ShortDash;
		if (ShortDashMontage)
			PlayAnimMontage(ShortDashMontage);
	}
	// 순간 발사
	LaunchCharacter(DashDir_ * Speed, true, false);
}

void AEnemyHuman::PlayTakeOffEffects()
{
	// ==================
	// 1) SFX
	// ==================
	if (TakeOffSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			TakeOffSFX,
			GetActorLocation()
		);
	}

	// ==================
	// 2) 캐릭터 발 아래 Dust VFX
	// ==================
	if (TakeOffDustVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			TakeOffDustVFX,
			GetActorLocation() - FVector(0,0,90.f)
		);
	}

	// Cascade 버전도 원하면 같은 식으로 추가
	if (TakeOffDustParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			TakeOffDustParticle,
			GetActorLocation() - FVector(0,0,90.f)
		);
	}

	// ==================
	// 3) 짧은 충격파(Shockwave) VFX
	// ==================
	if (TakeOffShockVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			TakeOffShockVFX,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}
}

/* ===========================
 *  TakeOff (위로 수직 점프)
 * ===========================
 */
void AEnemyHuman::StartTakeOff(float UpSpeed, float Duration)
{
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		TakeOffOriginalGravity = Move->GravityScale;

		// ★ 순간 상승
		LaunchCharacter(FVector(0.f, 0.f, UpSpeed), true, true);
		
	}
}


// ========================================================================
// ========================================================================
// ============================= Effects ==================================
// ========================================================================
// ========================================================================

void AEnemyHuman::PlayDashEffects(const FVector& DashDirection)
{
	// ============================
	// 1) Dash SFX
	// ============================
	if (DashSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			DashSFX,
			GetActorLocation()
		);
	}

	// ============================
	// 2) Dash VFX (Niagara)
	// ============================
	if (DashVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DashVFX,
			GetActorLocation(),
			DashDir.Rotation()
		);
	}

	// ============================
	// 2-1) Dash VFX (Cascade)
	// ============================
	if (DashParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DashParticle,
			GetActorTransform().GetLocation(),
			DashDir.Rotation()
		);
	}

	// ============================
	// 3) Ghost Trail (Niagara)
	// ============================
	if (GhostTrailVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAttached(
			GhostTrailVFX,
			GetMesh(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// ============================
	// 3-1) Ghost Trail (Cascade)
	// ============================
	if (GhostTrailParticle)
	{
		UGameplayStatics::SpawnEmitterAttached(
			GhostTrailParticle,
			GetMesh(),
			NAME_None,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true
		);
	}

	// ============================
	// 4) Foot Sliding Dust (Niagara)
	// ============================
	if (DashDustVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			DashDustVFX,
			GetActorLocation() - DashDir * 30.f
		);
	}

	// ============================
	// 4-1) Foot Sliding Dust (Cascade)
	// ============================
	if (DashDustParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			DashDustParticle,
			GetActorLocation() - DashDir * 30.f,
			FRotator::ZeroRotator
		);
	}
}
void AEnemyHuman::PlayBladeAppearSFX()
{
	if (BladeAppearSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BladeAppearSFX,
			GetActorLocation()
		);
	}
}
void AEnemyHuman::PlayBladeSwingSFX()
{
	if (BladeSwingSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BladeSwingSFX,
			GetActorLocation()
		);
	}
}
