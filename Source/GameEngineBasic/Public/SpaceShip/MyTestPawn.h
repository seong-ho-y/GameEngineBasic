#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystem.h"
#include "MyTestPawn.generated.h"

// 전방 선언
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;

class UHealthComp;
class UMyShipMovement;
class UShooterComp;

UCLASS()
class GAMEENGINEBASIC_API AMyTestPawn : public APawn
{
	GENERATED_BODY()

public:
	AMyTestPawn();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShipMesh;

	// 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* ShieldComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UHealthComp* HealthComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UMyShipMovement* ShipMovement;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UShooterComp* Shooter;


	// 입력
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_MoveForward;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Roll;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Look;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Boost;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Fire;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* IA_Brake;

	// 캐스케이드
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionFX = nullptr;

protected:
	// Projectile과 Overlap되었을 때
	UFUNCTION()
	void OnShieldOverlap(UPrimitiveComponent* Overlapped, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Ship Mesh가 Projectile제외 충돌했을 때
	UFUNCTION()
	void OnShipHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	UFUNCTION()
	void OnShieldBroken(AActor* OwnActor);

	// 체력/실드 값 변화 시 (HUD 갱신 등에 사용)
	UFUNCTION()
	void OnHealthChanged(AActor* OwnActor, float NewHealth, float NewShield);

	// 사망 처리
	UFUNCTION()
	void OnDeath(AActor* OwnActor);

protected:
	// 게임 시작 시 호출되는 함수
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	void MoveForward(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Roll(const FInputActionValue& Value);

	void Boost_Pressed();
	void Boost_Released();

	void Brake_Pressed();
	void Brake_Released();

	void FireTriggered(const FInputActionValue& Value);   // 연사
	void FireStarted(const FInputActionValue& Value);     // 단발
	void FireCompleted(const FInputActionValue& Value);   
};
