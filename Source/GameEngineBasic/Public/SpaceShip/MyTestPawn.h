#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SphereComponent.h"
#include "Particles/ParticleSystem.h"
#include "Item/U_Interactable.h"
#include "MyTestPawn.generated.h"

// 전방 선언
class USceneComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UHealthComp;
class UShieldComp;
class UShooterComp;
class AProjectile;

class UInputAction;
class UInputMappingContext;
class UMyShipMovement;
class ASpaceCharacter;

UCLASS()
class GAMEENGINEBASIC_API AMyTestPawn : public APawn, public IU_Interactable
{
	GENERATED_BODY()

public:
	AMyTestPawn();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ShipMesh;

	// Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UShieldComp* ShieldComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UHealthComp* HealthComp = nullptr;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UMyShipMovement* ShipMovement;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UShooterComp* Shooter;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USphereComponent* CollisionComp = nullptr;


	// Input
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

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* IA_Shield;

	// 캐스케이드
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionFX = nullptr;

	// Interactable
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TScriptInterface<IU_Interactable> CurrentInteractTarget;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile>  BaseProjectileClass;
public:
	FORCEINLINE class UShieldComp* GetShieldComponent() const { return ShieldComp; }
	FORCEINLINE class UHealthComp* GetHealthComponent() const { return HealthComp; }
	FORCEINLINE class UShooterComp* GetShooterComponent() const { return Shooter; }

public:
	UFUNCTION()
	void OnShieldBroken(AActor* OwnActor);

	// 체력/실드 값 변화 시 (HUD 갱신 등에 사용)
	UFUNCTION()
	void OnHealthChanged(float NewHealth, float MaxHealth);

	// 사망 처리
	UFUNCTION()
	void OnDeath(AActor* OwnActor);

	// Shield
	UFUNCTION()
	void OnShieldActivated();

	UFUNCTION()
	void OnShieldKeyPressed(const FInputActionInstance& Instance);

	// Overlap
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex);

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

	virtual void Interact(ASpaceCharacter* Character) override;
};
