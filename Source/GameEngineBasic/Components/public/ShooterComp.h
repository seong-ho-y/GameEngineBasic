// It's a component defines weapon. It has a Logic for Fire. Also, you can assign a projectile class.
// Enemy and Player(Actor that has shoot weapon) should have this component
// You can override CanFire() and Fire() in BluePrint.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Projectile.h"
#include "ShooterComp.generated.h"

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Rifle      UMETA(DisplayName = "Rifle"),
	Missile    UMETA(DisplayName = "Missile"),
	Laser      UMETA(DisplayName = "Laser")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAmmoChanged, int32, CurrentAmmo, int32, FullAmmo, int32, MaxAmmo);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class GAMEENGINEBASIC_API UShooterComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UShooterComp();

	UPROPERTY(BlueprintAssignable, Category="Events")
	FOnAmmoChanged OnAmmoChanged;
	bool bHasExternalMuzzleInfo = false;
	FVector ExternalMuzzleLoc;
	FRotator ExternalMuzzleRot;
	float ReloadTime = 5.f;
	
	UFUNCTION()
	void HandleWeaponInitialized();
protected:

	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	bool TryFire();

	UFUNCTION(BlueprintNativeEvent)
	void Fire();

	UFUNCTION(BlueprintCallable)
	void SetFireDirection(const FVector& NewDir);

	UFUNCTION(BlueprintCallable)
	bool CanFire() const;

	UFUNCTION()
	void ResetFireReady();


	UFUNCTION()
	void SetProjectile(); // ProjectileMap에서 자동 선택

	UFUNCTION(BlueprintCallable)
	void StartReload();

	UFUNCTION(BlueprintCallable)
	void ReloadSuccess();
	void SetMuzzle(const FVector& Loc);
	void ClearMuzzle();
	FVector FindMuzzleLoc() const;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TMap<EProjectileType, TSubclassOf<AProjectile>> ProjectileMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EProjectileType CurrentProjectileType = EProjectileType::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName MuzzleSocketName = "Muzzle";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 CurrentAmmo = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 FullAmmo = 30;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 MaxAmmo = 999;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float PendingDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float PendingScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UNiagaraSystem* MuzzleFlashEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FVector FireDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FVector MuzzleLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Homing")
	bool bUseArcHoming = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Target")
	AActor* CurrentTarget = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Option")
	bool bUseAmmo = true;

	UPROPERTY()
	float ReloadTimeRemaining = 0.f;

	UPROPERTY()
	float ReloadTimeTotal = 0.f;
	
	UPROPERTY()
	bool bIsReadyToFire = true;
private:
	

	bool bIsReloading = false;

	FTimerHandle FireRateTimerHandle;
	FTimerHandle ReloadTimerHandle;
public:

};
