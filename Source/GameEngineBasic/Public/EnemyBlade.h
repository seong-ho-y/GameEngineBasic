// EnemyBlade.h

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyBlade.generated.h"

class USceneComponent;
class UBoxComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class ACharacter;

UCLASS()
class GAMEENGINEBASIC_API AEnemyBlade : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemyBlade();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// =======================
	//      Components
	// =======================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UBoxComponent* BladeCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* BladeMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UNiagaraComponent* TrailVFX;

	// =======================
	//      VFX / Combat
	// =======================
	// 히트 스파크 이펙트 (Optional)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="VFX")
	UNiagaraSystem* HitSparkVFX;

	// 한 번 맞았을 때 줄 기본 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	float Damage = 20.f;

	// 박스 히트박스 기본 크기 (앞 / 좌우 / 상하)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	FVector BoxExtent = FVector(60.f, 20.f, 20.f);

	// 박스 회전 (필요하면 BP에서 손에 맞게 수정)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
	FRotator BoxRotation = FRotator(0.f, 0.f, 0.f);

	// 디버그용 히트박스 그리기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
	bool bDebugHitbox = false;

	// 이 블레이드를 휘두르는 소유 캐릭터 (EnemyHuman 등)
	UPROPERTY(BlueprintReadOnly, Category="Owner")
	ACharacter* OwnerCharacter = nullptr;

	// 한 번 스윙 동안 이미 맞은 액터 (중복 히트 방지)
	UPROPERTY()
	TSet<AActor*> AlreadyHitActors;

	// ==============
	//  Public API
	// ==============
	// 공격 시작 시 (AnimNotify_MeleeBegin 같은 데서 호출)
	UFUNCTION(BlueprintCallable, Category="Combat")
	void ActivateHitbox();

	// 공격 종료 시 (AnimNotify_MeleeEnd 같은 데서 호출)
	UFUNCTION(BlueprintCallable, Category="Combat")
	void DeactivateHitbox();

	// 필요하면 외부에서 콤보 시작 시 초기화 가능
	UFUNCTION(BlueprintCallable, Category="Combat")
	void ResetHitList();

protected:
	void StartTrail();
	void StopTrail();

	// Overlap 이벤트
	UFUNCTION()
	void OnBladeOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};
