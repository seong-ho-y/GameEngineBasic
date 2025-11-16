#include "HomingMissileProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "TimerManager.h"

AHomingMissileProjectile::AHomingMissileProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    ProjectileMovement->InitialSpeed = HomingSpeed;
    ProjectileMovement->MaxSpeed = HomingSpeed;
    ProjectileMovement->ProjectileGravityScale = 0.f;
    ProjectileMovement->bIsHomingProjectile = false; // 커스텀 유도 로직 사용
}

void AHomingMissileProjectile::BeginPlay()
{
    Super::BeginPlay();
   /* UE_LOG(LogTemp, Warning, TEXT("[Missile] BeginPlay HomingType=%d (0=Arc,1=Direct) | Owner=%s"),
        (int32)HomingType,
        *GetOwner()->GetName());
    */
    CachedStartZ = GetActorLocation().Z;

    // 일정 시간 뒤 폭발
    GetWorldTimerManager().SetTimer(
        ExplosionTimerHandle,
        this,
        &AHomingMissileProjectile::Explode,
        ExplosionDelay,
        false
    );
    GetWorldTimerManager().SetTimerForNextTick(this, &AHomingMissileProjectile::InitializeHomingMode);
}
void AHomingMissileProjectile::InitializeHomingMode()
{
    // 이제 ShooterComp이 HomingType을 세팅한 이후이므로 안전함
    if (HomingType == EHomingType::ArcHoming)
    {
        bIsArcPhase = true;
        FVector InitVel = GetActorForwardVector() * HomingSpeed + FVector(0, 0, ArcHeight);
        ProjectileMovement->Velocity = InitVel;
        //UE_LOG(LogTemp, Warning, TEXT("[Arc] InitVel: %s"), *InitVel.ToString());
    }
    else
    {
        bIsArcPhase = false;
        ProjectileMovement->Velocity = GetActorForwardVector() * HomingSpeed;
        GetWorldTimerManager().SetTimer(
            GravityTimerHandle,
            this,
            &AHomingMissileProjectile::EnableGravity,
            GravityEnableDelay,
            false);
    }

    //UE_LOG(LogTemp, Warning, TEXT("Missile Projectile Mode Start (HomingType=%d)"), (int32)HomingType);
}

void AHomingMissileProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!HomingTarget) return;

    FVector ToTarget = (HomingTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector OldVel = ProjectileMovement->Velocity;
    FVector OldDir = OldVel.GetSafeNormal();

    FVector NewVelocity = OldVel;

    // ===========================================================
    // 1) ARC / DIRECT 기본 방향 계산
    // ===========================================================
    bool bAllowHoming = true;

    if (HomingType == EHomingType::ArcHoming)
    {
        if (bIsArcPhase)
        {
            if (GetActorLocation().Z - CachedStartZ >= ArcHeight * 0.8f)
                bIsArcPhase = false;
            bAllowHoming = !bIsArcPhase;
        }
    }

    // ===========================================================
    // 2) 가속도 적용 (AccelSpeed > 0일 때만)
    // ===========================================================
    if (bAllowHoming)
    {
        if (AccelSpeed > 0.f)
        {
            // 방향은 기존 OldDir 유지, 속력만 증가
            float CurrentSpeed = OldVel.Size();
            float NewSpeed = CurrentSpeed + (AccelSpeed * DeltaSeconds);
            

            FVector NewDir =
                FMath::VInterpTo(OldDir, ToTarget, DeltaSeconds, TurnInterpSpeed).GetSafeNormal();

            NewVelocity = NewDir * NewSpeed;
        }
        else
        {
            // AccelSpeed == 0 → 기존 방식
            FVector NewDir =
                FMath::VInterpTo(OldDir, ToTarget, DeltaSeconds, TurnInterpSpeed).GetSafeNormal();
            NewVelocity = NewDir * HomingSpeed;
        }
    }

    // ===========================================================
    // 3) 중력 중이면 방향 변경 X (Direct Homing 전용)
    // ===========================================================
    if (HomingType == EHomingType::DirectHoming &&
        ProjectileMovement->ProjectileGravityScale > 0.f)
    {
        // 중력 떨어지는 동안에는 유도 중지
        NewVelocity = OldVel;
    }

    // 적용
    ProjectileMovement->Velocity = NewVelocity;

    // 회전 보정
    SetActorRotation(ProjectileMovement->Velocity.Rotation());
}


void AHomingMissileProjectile::SetHomingTarget(AActor* Target)
{
    HomingTarget = Target;
}

void AHomingMissileProjectile::SetHomingType(EHomingType NewType)
{
    HomingType = NewType;
}

void AHomingMissileProjectile::EnableGravity()
{
    if (!ProjectileMovement) return;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
}

void AHomingMissileProjectile::Explode()
{
    // 폭발 이펙트
    if (ExplosionVfx)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ExplosionVfx, GetActorLocation());
        if (ExplosionSound) UGameplayStatics::SpawnSoundAtLocation(GetWorld(), ExplosionSound, GetActorLocation());
    }
    /*if (ImpactEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            ImpactEffect,
            GetActorLocation(),
            GetActorRotation());
    }
    */
    // 폭발 데미지
    UGameplayStatics::ApplyRadialDamage(
        this,
        DamageAmount,
        GetActorLocation(),
        300.f,
        UDamageType::StaticClass(),
        {},
        this,
        GetOwner() ? GetOwner()->GetInstigatorController() : nullptr,
        true
    );

    Destroy();
}

void AHomingMissileProjectile::OnHit_Implementation(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::OnHit_Implementation(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

    if (OtherActor == this || OtherActor == GetOwner()) return;

    //UE_LOG(LogTemp, Warning, TEXT("[Missile] OnHit -> %s"), *OtherActor->GetName());
    Explode();
}
