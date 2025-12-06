#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"

#include "WeaponComponent.generated.h"

class AProjectile;
class ASpaceCharacter;
class UShooterComp;
class UWeaponBehavior;           // ★ Behavior 베이스
class UPrimaryWeaponBehavior;    // ★ Primary용
class UChargeWeaponBehavior;     // ★ Charge용


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    Base,
    Rifle,         // 연사
    Charge,     // 차지
	ShotGun	// 샷건
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()
	

	/* --------------------------- 기본 정보 --------------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EWeaponType WeaponType = EWeaponType::Base;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* WeaponMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector MeshScale = FVector(1.f, 1.f, 1.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector LocPivot = FVector(0.f, 0.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FRotator RotPivot = FRotator(0.f, 0.f, 0.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	/* --------------------------- 스탯 --------------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.1f; // 연사력 or 발사 쿨다운

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReloadTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxAmmo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FullAmmo = 30;

	/* --------------------------- 사거리 / 투사체 --------------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsAuto;
	

	/* --------------------------- 차지형 무기 옵션 --------------------------- */

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsChargeWeapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChargeTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float OverchargeTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* ChargeVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* ChargeSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* OverChargeVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* OverChargeSFX;

	/* ------------------------ 스프레드 옵션 -------------------------*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SpreadCount = 1; // 한 번에 발사할 탄 수

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpreadAngle = 5.f; // 탄 퍼짐 각도
};

USTRUCT(BlueprintType)
struct FWeaponRuntimeState
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CurrentAmmo = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ReserveAmmo = 0;
	
};
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeaponInitialized);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

	UPROPERTY()
	FOnWeaponInitialized WeaponInitialized;

	UPROPERTY()
	UWeaponBehavior* Behavior;

	void InitializeWeapon(ASpaceCharacter* Player, UShooterComp* InShooterComp);
	

	virtual void HandleFirePressed();
	virtual void HandleFireReleased();
	
	FVector GetAimPoint() const;
	FVector GetMuzzleLoc() const;
	void SpawnAndAttachWeaponMesh();
	void SaveRuntimeState();
	void ApplyWeaponStatsToShooter();
	void ClearWeaponMesh();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon Data")
	UDataTable* WeaponTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category="Weapon Data")
	FName WeaponRowName;

	UPROPERTY()
	FWeaponRuntimeState RuntimeState;

	
protected:

	virtual void BeginPlay() override;


	UPROPERTY()
	UStaticMeshComponent* WeaponMeshComp = nullptr;
	
	UPROPERTY()
	UWeaponComponent* CurrentWeaponBehavior;
	
	// Behavior 클래스 선택/생성용 헬퍼
	void SetupBehaviorFromData();

	// Behavior를 직접 세팅하고 Initialize까지 해주는 함수 (원하면 public로 빼도 됨)
	void SetBehavior(TSubclassOf<UWeaponBehavior> BehaviorClass);
	
	

public:
	UPROPERTY()
	ASpaceCharacter* OwnerCharacter;
	
	UPROPERTY()
	UShooterComp* ShooterComp;
	
	UPROPERTY(EditAnywhere)
	FWeaponData WeaponData;
	
	UPROPERTY(EditAnywhere, Category = "Muzzle")
	FName MuzzleSocketName = "MuzzleSocket";
	
	virtual FVector GetAimDirection() const; // If there are more than one aim logic, you can override in child weapon

};
