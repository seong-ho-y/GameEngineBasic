#include "ChargeWeaponComponent.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "SpaceCharacter/SpaceCharacter.h"


UChargeWeaponComponent::UChargeWeaponComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UChargeWeaponComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UChargeWeaponComponent::HandleFirePressed()
{
    if (!ShooterComp || !OwnerCharacter) return;

    bIsCharging = true;
    CurrentChargeTime = 0.f;

    // 차지 증가 타이머
    GetWorld()->GetTimerManager().SetTimer(
        ChargeTickHandle,
        [this]()
        {
            CurrentChargeTime += 0.02f;

            // 과충전 체크
            if (WeaponData.bIsChargeWeapon && 
                CurrentChargeTime >= WeaponData.OverchargeTime)
            {
                GetWorld()->GetTimerManager().ClearTimer(ChargeTickHandle);
                OverchargeExplode();
            }

        },
        0.02f,
        true
    );
}

void UChargeWeaponComponent::HandleFireReleased()
{
    if (!bIsCharging)
        return;

    bIsCharging = false;
    GetWorld()->GetTimerManager().ClearTimer(ChargeTickHandle);

    ReleaseCharge();
}

void UChargeWeaponComponent::StartCharge()
{
    // 필요시 모션/이펙트 시작
}

void UChargeWeaponComponent::ReleaseCharge()
{
    if (!ShooterComp || !OwnerCharacter) return;

    float ChargeRatio = FMath::Clamp(
        CurrentChargeTime / WeaponData.ChargeTime,
        0.f,
        1.f
    );

    float FinalDamage = WeaponData.Damage * (1.f + ChargeRatio);

    ShooterComp->PendingDamage = FinalDamage;
    ShooterComp->ProjectileClass = WeaponData.ProjectileClass;

    ShooterComp->SetFireDirection(GetAimDirection());
    ShooterComp->TryFire();

    // 사운드/이펙트/모션 가능
}

void UChargeWeaponComponent::OverchargeExplode()
{
    UE_LOG(LogTemp, Warning, TEXT("Overcharge!!!"));

    // 플레이어 데미지
    if (OwnerCharacter)
    {
        UGameplayStatics::ApplyDamage(OwnerCharacter, 50.f, nullptr, nullptr, nullptr);
    }

    // 과충전 폭발 이펙트
    if (OverchargeVFX) 
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverchargeVFX,
            OwnerCharacter->GetActorLocation());
    }
}
