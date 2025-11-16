#include "TurretLaserComponent.h"


#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UTurretLaserComponent::UTurretLaserComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretLaserComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!LaserSystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurretLaserComp: LaserSystem is not set on %s"), *GetOwner()->GetName());
		return;
	}

	// Turret 메시에 붙여서 나이아가라 생성
	USkeletalMeshComponent* MeshComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("TurretLaserComp: No SkeletalMeshComponent found on %s"), *GetOwner()->GetName());
		return;
	}

	LaserComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		LaserSystem,
		MeshComp,
		LaserStartSocketName,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		EAttachLocation::SnapToTarget,
		false  // bAutoDestroy
	);

	if (LaserComp)
	{
		LaserComp->SetAutoActivate(false); // 시작은 비활성
	}
	
}

void UTurretLaserComponent::TickComponent(
	float DeltaTime,
	enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateLaserFX(DeltaTime);
}

void UTurretLaserComponent::SetLaserEnabled(bool bEnable)
{
	bWantsLaser = bEnable;
}

void UTurretLaserComponent::SetCurrentTarget(AActor* NewTarget)
{
	CurrentTarget = NewTarget;
}

void UTurretLaserComponent::OnStunStateChanged(bool bStunned)
{
	bIsStunned = bStunned;
}

void UTurretLaserComponent::UpdateLaserFX(float DeltaTime)
{
	if (!LaserComp)
		return;

	// 1) 레이저를 켜야 하는지 판단
	bool bShouldBeActive = bWantsLaser && !bIsStunned && (CurrentTarget != nullptr);

	if (!bShouldBeActive)
	{
		ApplyLaserActiveState(false);
		return;
	}

	// 2) 시작 위치 계산 (소켓 기준)
	USkeletalMeshComponent* MeshComp = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
	if (!MeshComp)
	{
		ApplyLaserActiveState(false);
		return;
	}

	const FVector StartLoc = MeshComp->GetSocketLocation(LaserStartSocketName);

	// 3) 타겟 위치 계산 (플레이어)
	const FVector TargetLoc = CurrentTarget->GetActorLocation();
	FVector Dir = TargetLoc - StartLoc;

	const float Dist = Dir.Length();
	if (Dist < MinLaserDistance)
	{
		// 너무 가까우면 레이저 끔
		ApplyLaserActiveState(false);
		return;
	}

	Dir = Dir.GetSafeNormal();
	FVector EndLoc = TargetLoc;

	// 우선은 플레이어까지 트레이스
	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TurretLaserTrace), false, GetOwner());
	Params.AddIgnoredActor(GetOwner());         // 터렛 자기 자신 무시
	Params.AddIgnoredActor(CurrentTarget);      // 플레이어도 기본 트레이스로는 무시

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartLoc,
		TargetLoc,
		ECC_Visibility,
		Params
	);

	// 트레이스 결과가 “벽/지형” 등과 충돌하면 EndLoc 변경
	if (bHit)
	{
		EndLoc = HitResult.ImpactPoint;
		UE_LOG(LogTemp, Warning, TEXT("Laser hit: %s at %s"),
	   *HitResult.GetActor()->GetName(),
	   *HitResult.ImpactPoint.ToString());
	}

	// 5) 나이아가라 변수 업데이트
	ApplyLaserActiveState(true);

	// 나이아가라에서 사용할 변수 이름은 네가 만든 System에 맞게 변경해야 함
	LaserComp->SetWorldLocation(StartLoc);
	LaserComp->SetVariableVec3(TEXT("Beam End"), EndLoc);
	// 예: "User.End", "User.Target" 등으로 되어있으면 그 이름으로 변경
}

void UTurretLaserComponent::ApplyLaserActiveState(bool bShouldBeActive)
{
	if (!LaserComp)
		return;

	if (bShouldBeActive && !bLaserActive)
	{
		LaserComp->Activate();
		bLaserActive = true;
	}
	else if (!bShouldBeActive && bLaserActive)
	{
		LaserComp->Deactivate();
		bLaserActive = false;
	}
}
