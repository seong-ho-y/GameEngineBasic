#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Projectile.h"
#include "ShooterComp.generated.h"

UENUM(BlueprintType)
enum class EProjectileType : uint8
{
	Rifle      UMETA(DisplayName="Rifle"),
	Missile    UMETA(DisplayName="Missile"),
	Laser      UMETA(DisplayName="Laser")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAMEENGINEBASIC_API UShooterComp : public UActorComponent
{
	GENERATED_BODY()

public:
	UShooterComp();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// =====================
	// 기본 발사 기능
	// =====================

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

	// =====================
	// Projectile 설정
	// =====================

	UFUNCTION()
	void SetProjectile(); // ProjectileMap에서 자동 선택

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	TMap<EProjectileType, TSubclassOf<AProjectile>> ProjectileMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	EProjectileType CurrentProjectileType = EProjectileType::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FName MuzzleSocketName = "Muzzle";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float FireRate = 0.01f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	int32 CurrentAmmo = 999;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float PendingDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	float PendingScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	UNiagaraSystem* MuzzleFlashEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon")
	FVector FireDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Weapon|Homing")
	bool bUseArcHoming = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="Weapon|Target")
	AActor* CurrentTarget = nullptr;
private:
	UPROPERTY()
	bool bIsReadyToFire = true;

	FTimerHandle FireRateTimerHandle;
};
