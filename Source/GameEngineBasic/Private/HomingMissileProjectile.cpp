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
    FVector CurrentVel = ProjectileMovement->Velocity.GetSafeNormal();

    if (HomingType == EHomingType::ArcHoming)
    {
        if (bIsArcPhase)
        {
            // 높이 도달 시 유도 시작
            if (GetActorLocation().Z - CachedStartZ >= ArcHeight * 0.8f)
            {
                bIsArcPhase = false;
            }
        }

        // 유도 중이면 방향 보간
        if (!bIsArcPhase)
        {
            FVector NewDir = FMath::VInterpTo(CurrentVel, ToTarget, DeltaSeconds, TurnInterpSpeed).GetSafeNormal();
            ProjectileMovement->Velocity = NewDir * HomingSpeed;
        }
    }
    else // Direct Homing
    {
        if (ProjectileMovement->ProjectileGravityScale == 0.f)
        {
            FVector NewDir = FMath::VInterpTo(CurrentVel, ToTarget, DeltaSeconds, TurnInterpSpeed).GetSafeNormal();
            ProjectileMovement->Velocity = NewDir * HomingSpeed;
        }
    }

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
