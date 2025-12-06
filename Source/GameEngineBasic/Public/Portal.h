
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Portal.generated.h"


class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
class UNiagaraComponent;

UCLASS()
class GAMEENGINEBASIC_API APortal : public AActor
{
	GENERATED_BODY()
	
public:	
	APortal();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:	
	UPROPERTY(VisibleAnywhere, Category = "Portal")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category = "Portal")
	USphereComponent* CollisionSphere;

	UPROPERTY(VisibleAnywhere, Category = "Portal")
	UStaticMeshComponent* Cylinder;

	UPROPERTY(VisibleAnywhere, Category = "Portal")
	UNiagaraComponent* Effect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Portal")
	FName DestinationLevelName;

public:
	UFUNCTION()
	void OnPortalOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
