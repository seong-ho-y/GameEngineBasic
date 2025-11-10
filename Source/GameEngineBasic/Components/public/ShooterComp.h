// It's a component defines weapon. It has a Logic for Fire. Also, you can assign a projectile class.
// Enemy and Player(Actor that has shoot weapon) should have this component
// You can override CanFire() and Fire() in BluePrint.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "Projectile.h"
#include "ShooterComp.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMEENGINEBASIC_API UShooterComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShooterComp();
	
	// 발사할 프로젝타일 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Data")
	TSubclassOf<AProjectile> ProjectileClass;
    
	// 발사 속도 (초당 발사 횟수의 역수, 예: 0.1f = 초당 10발)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Data")
	float FireRate = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Data")
	float PendingDamage = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Data")
	float PendingScale = 1.f;

	// 탄약
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Data")
	int32 MaxAmmo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Data")
	int32 CurrentAmmo;

	// 총구 소켓 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Data")
	FName MuzzleSocketName = "Muzzle";

	// 총구 발사 이펙트
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Effects")
	UNiagaraSystem* MuzzleFlashEffect;

	// 발사 사운드
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shoot|Effects")
	USoundBase* FireSound;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool TryFire();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	bool CanFire() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Combat")
	void Fire();
	
	UFUNCTION(BlueprintCallable)
	void SetFireDirection(const FVector& NewDir);

private:
	FVector FireDirection = FVector::ZeroVector;
	
	// 연사 속도 제어를 위한 타이머 핸들
	FTimerHandle FireRateTimerHandle;

	// 현재 발사 가능한 상태인지 나타내는 플래그
	bool bIsReadyToFire = true;

	// 쿨다운이 끝나면 bIsReadyToFire를 true로 되돌리는 함수
	void ResetFireReady();

public:
	
};
