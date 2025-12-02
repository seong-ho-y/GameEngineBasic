#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBlade.generated.h"

class UBoxComponent;
class UCapsuleComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UCLASS(config = Game)
class GAMEENGINEBASIC_API AEnemyBlade : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemyBlade();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// ======================
	//  Components
	// ======================
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* RootScene;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* BladeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UBoxComponent* BladeCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UNiagaraComponent* TrailVFX;  // 계속되는 트레일

	// ======================
	//  Parameters
	// ======================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float Damage = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float HitboxLength = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float HitboxRadius = 20.f;

	// Owner (EnemyHuman)
	UPROPERTY()
	AActor* OwnerCharacter;

	// Multiple-hit prevention
	UPROPERTY()
	TArray<AActor*> AlreadyHitActors;


	// ======================
	//  Methods
	// ======================
	void ActivateHitbox();     // AnimNotify Begin
	void DeactivateHitbox();   // AnimNotify End
	void ResetHitList();

	// VFX
	void StartTrail();
	void StopTrail();

	// Spark VFX on hit
	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* HitSparkVFX;

	// Trail VFX template
	UPROPERTY(EditAnywhere, Category="VFX")
	UNiagaraSystem* TrailFX;

	// Collision Overlap
	UFUNCTION()
	void OnBladeOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};
