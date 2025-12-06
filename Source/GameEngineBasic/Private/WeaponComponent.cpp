// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponComponent.h"

#include "WeaponBehavior.h"
#include "PrimaryWeaponBehavior.h"
#include "ChargeWeaponBehavior.h"
#include "MultiWeaponBehavior.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "SpaceCharacter/SpaceCharacter.h"


// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();
	/*GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green,
	FString::Printf(TEXT("WeaponComp Owner = %s"),
	*GetOwner()->GetName()));*/
}


void UWeaponComponent::InitializeWeapon(ASpaceCharacter* Player, UShooterComp* InShooterComp)
{
	OwnerCharacter = Player;
	ShooterComp = InShooterComp;

	// 1) DataTable에서 무기 데이터 로드
	if (WeaponTable && WeaponRowName != NAME_None)
	{
		if (const FWeaponData* Row = WeaponTable->FindRow<FWeaponData>(WeaponRowName, TEXT("InitializeWeapon")))
			{
				WeaponData = *Row;
			}
		}
	else
	{
		/*const FString TableName = WeaponTable ? WeaponTable->GetName() : TEXT("NULL");
		GEngine->AddOnScreenDebugMessage(
				-1, 6.f, FColor::Red,
				FString::Printf(
					TEXT("NoWeapon → Table=%s | RowName=%s"),
					*TableName,
					*WeaponRowName.ToString()));*/
		return;
	}

	// 2) RuntimeState (탄약 상태) 로드 또는 초기화
	if (OwnerCharacter->WeaponStates.Contains(WeaponRowName))
		{
			RuntimeState = OwnerCharacter->WeaponStates[WeaponRowName];
		}
	else
		{
			RuntimeState.CurrentAmmo = WeaponData.FullAmmo;
			RuntimeState.ReserveAmmo = WeaponData.MaxAmmo;

			OwnerCharacter->WeaponStates.Add(WeaponRowName, RuntimeState);
		}

		// 3) ShooterComp에 값 적용 (Damage, FireRate, Ammo 등)
	ApplyWeaponStatsToShooter();

		// 4) Behavior 선택/생성 (★ 새로 추가되는 부분)
	SetupBehaviorFromData();

		// 5) 무기 Mesh 스폰/부착
	SpawnAndAttachWeaponMesh();

		// 6) HUD 업데이트 브로드캐스트
	WeaponInitialized.Broadcast();
}


void UWeaponComponent::SetBehavior(TSubclassOf<UWeaponBehavior> BehaviorClass)
{
	if (!BehaviorClass)
	{
		Behavior = nullptr;
		return;
	}

	// 같은 클래스면 그냥 재초기화만 해도 됨
	if (Behavior && Behavior->GetClass() == BehaviorClass)
	{
		Behavior->Initialize(this);
		return;
	}

	// 새 Behavior 생성
	Behavior = NewObject<UWeaponBehavior>(this, BehaviorClass);
	if (Behavior)
	{
		Behavior->Initialize(this);
	}
}

void UWeaponComponent::SetupBehaviorFromData()
{
	// WeaponData.WeaponType 기반으로 선택
	TSubclassOf<UWeaponBehavior> BehaviorClass = nullptr;

	switch (WeaponData.WeaponType)
	{
	case EWeaponType::Base:
	case EWeaponType::Rifle:
		BehaviorClass = UPrimaryWeaponBehavior::StaticClass();
		break;

	case EWeaponType::Charge:
		BehaviorClass = UChargeWeaponBehavior::StaticClass();
		break;

	case EWeaponType::ShotGun:
		BehaviorClass = UMultiWeaponBehavior::StaticClass();
		break;

	default:
		BehaviorClass = nullptr;
		break;
	}

	SetBehavior(BehaviorClass);
}


FVector UWeaponComponent::GetAimDirection() const
{
	if (!OwnerCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponComp : No Player Found!"));
		return FVector::ForwardVector;
	}
	if (!ShooterComp) return FVector::ForwardVector;
	const FVector AimPoint = GetAimPoint();
	const FVector MuzzleLoc = GetMuzzleLoc();
	ShooterComp->SetMuzzle(MuzzleLoc);
	return (AimPoint - MuzzleLoc).GetSafeNormal();
}


void UWeaponComponent::HandleFirePressed()
{
	if (Behavior)
	{
		Behavior->OnFirePressed();
	}
}

void UWeaponComponent::HandleFireReleased()
{
	if (Behavior)
	{
		Behavior->OnFireReleased();
	}
}



FVector UWeaponComponent::GetAimPoint() const
{
	if (!OwnerCharacter) return FVector::ZeroVector;

	APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
	if (!PC) return FVector::ZeroVector;

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);

	FVector CamOrigin;
	FVector CamDirection;

	if (!PC->DeprojectScreenPositionToWorld(
		SizeX * 0.5f,
		SizeY * 0.5f,
		CamOrigin,
		CamDirection))
	{
		return FVector::ZeroVector;
	}
	const float TraceDistance = 50000.f;
	FVector TraceEnd = CamOrigin + CamDirection * TraceDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerCharacter); 

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		CamOrigin,
		TraceEnd,
		ECC_Visibility,
		Params);
	
	FVector AimPoint = bHit ? Hit.Location : TraceEnd;

	// TPS 근거리 보정 (필수!)
	float Dist = FVector::Dist(CamOrigin, AimPoint);
	if (Dist < 150.f)  // 벽 1.5m 안쪽일 때
	{
		AimPoint = CamOrigin + CamDirection * 2000.f;
	}

	return AimPoint;
}

FVector UWeaponComponent::GetMuzzleLoc() const
{
	// 1) 무기 메쉬가 있으면 무기 메쉬 소켓 사용
	if (WeaponMeshComp)
	{
		return WeaponMeshComp->GetSocketLocation(MuzzleSocketName);
	}

	// 2) 없으면 플레이어 메쉬 fallback
	if (OwnerCharacter && OwnerCharacter->GetMesh())
	{
		return OwnerCharacter->GetMesh()->GetSocketLocation(MuzzleSocketName);
	}

	return FVector::ZeroVector;
}

void UWeaponComponent::SpawnAndAttachWeaponMesh()
{
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
		return;

	if (!WeaponData.WeaponMesh)   // DataTable에 SkeletalMesh 또는 StaticMesh 넣어두기
		return;

	if (!WeaponMeshComp)
	{
		WeaponMeshComp = NewObject<UStaticMeshComponent>(OwnerCharacter);
		WeaponMeshComp->RegisterComponent();
	}

	WeaponMeshComp->SetStaticMesh(WeaponData.WeaponMesh);


	// 무기 소켓이름은 DT 또는 Blueprint로 지정
	FName SocketName = TEXT("WeaponSocket");

	WeaponMeshComp->AttachToComponent(
		OwnerCharacter->GetMesh(),
		FAttachmentTransformRules::SnapToTargetIncludingScale,
		SocketName
	);

	WeaponMeshComp->SetRelativeScale3D(WeaponData.MeshScale);
	WeaponMeshComp->SetRelativeLocationAndRotation(WeaponData.LocPivot, WeaponData.RotPivot);
	UE_LOG(LogTemp, Warning, TEXT("Scale: %s"),
	*WeaponData.MeshScale.ToString());
}


void UWeaponComponent::SaveRuntimeState()
{
	RuntimeState.CurrentAmmo = ShooterComp->CurrentAmmo;
	RuntimeState.ReserveAmmo = ShooterComp->MaxAmmo;
	OwnerCharacter->WeaponStates[WeaponRowName] = RuntimeState;
}


void UWeaponComponent::ApplyWeaponStatsToShooter()
{
	if (!ShooterComp) return;

	if (OwnerCharacter->WeaponStates.Contains(WeaponRowName))
	{
		ShooterComp->CurrentAmmo = OwnerCharacter->WeaponStates[WeaponRowName].CurrentAmmo;
		ShooterComp->MaxAmmo     = OwnerCharacter->WeaponStates[WeaponRowName].ReserveAmmo;
	}
	else
	{
		ShooterComp->CurrentAmmo = WeaponData.FullAmmo;
		ShooterComp->MaxAmmo = WeaponData.MaxAmmo;
	}
	ShooterComp->PendingDamage = WeaponData.Damage;
	ShooterComp->ReloadTime    = WeaponData.ReloadTime;
	ShooterComp->FireRate      = WeaponData.FireRate;
	ShooterComp->ProjectileClass = WeaponData.ProjectileClass;

	ShooterComp->FullAmmo    = WeaponData.FullAmmo;
}
void UWeaponComponent::ClearWeaponMesh()
{
	if (!OwnerCharacter || !OwnerCharacter->GetMesh())
		return;

	USkeletalMeshComponent* PlayerMesh = OwnerCharacter->GetMesh();

	// PlayerMesh의 모든 자식 컴포넌트 탐색
	TArray<USceneComponent*> Children;
	PlayerMesh->GetChildrenComponents(true, Children);

	for (USceneComponent* Child : Children)
	{
		// Weapon Mesh는 StaticMeshComponent 형태
		UStaticMeshComponent* SM = Cast<UStaticMeshComponent>(Child);
		if (!SM) continue;

		// 너가 붙인 무기 Mesh인지 판별
		// 이름으로 필터링 (WeaponMesh, GunMesh, etc)
		const FString Name = SM->GetName();

		if (Name.Contains(TEXT("Weapon")) ||
			Name.Contains(TEXT("Gun"))     ||
			SM == WeaponMeshComp)  // 기존 저장된 MeshComp 대비
		{
			SM->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			SM->DestroyComponent();
		}
	}

	// 레퍼런스도 정리
	WeaponMeshComp = nullptr;
}