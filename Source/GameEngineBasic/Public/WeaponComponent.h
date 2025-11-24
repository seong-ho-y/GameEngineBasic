#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"

#include "WeaponComponent.generated.h"

class AProjectile;
class UShooterComp;
class ASpaceCharacter;


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    None,
    Gatling,         // 기관총
    MachineGun,      // 고연사 머신건
    EnergyBlast,     // 에너지 블라스트 (차지형)
    LaserCannon,     // 레이저 캐논 (차지/관통/히트스캔)
    MissileLauncher, // 미사일 런처
    EnergyBlade      // 근접 블레이드
};

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
    GENERATED_BODY()

public:

    /* --------------------------- 기본 정보 --------------------------- */

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWeaponType WeaponType = EWeaponType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* WeaponMesh = nullptr;

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
	UParticleSystem* ChargeSFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* OverChargeVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* OverChargeSFX;
    /* --------------------------- 이동 디버프 (머신건/레이저캐논) --------------------------- */

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoveSpeedMultiplier = 1.0f; // 발사중 이속 감소 (예: 0.7)

    /* --------------------------- 미사일 옵션 --------------------------- */

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MissileCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bUseHoming = true;

    /* --------------------------- 검기 옵션 --------------------------- */

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasMeleeSlashWave = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<AProjectile> SlashProjectileClass;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

	void InitializeWeapon(ASpaceCharacter* Player, UShooterComp* InShooterComp);


	virtual void HandleFirePressed();
	virtual void HandleFireReleased();

	virtual bool CanFire() const;
	virtual void PerformFire();
	FVector GetAimPoint() const;
	FVector GetMuzzleLoc() const;
	void SpawnAndAttachWeaponMesh();

	UPROPERTY(EditAnywhere, Category="Weapon Data")
	UDataTable* WeaponTable;

	UPROPERTY(EditAnywhere, Category="Weapon Data")
	FName WeaponRowName;
protected:

	UPROPERTY()
	ASpaceCharacter* OwnerCharacter;

	UPROPERTY()
	UStaticMeshComponent* WeaponMeshComp = nullptr;
	
	UPROPERTY()
	UShooterComp* ShooterComp;

public:
	UPROPERTY(EditAnywhere)
	FWeaponData WeaponData;
	
	UPROPERTY(EditAnywhere, Category = "Muzzle")
	FName MuzzleSocketName = "MuzzleSocket";
	
	virtual FVector GetAimDirection() const; // If there are more than one aim logic, you can override in child weapon
public:	
	// Called every frame
	//virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void SetWeaponMesh(UStaticMeshComponent* InWeaponMeshComp);

};
