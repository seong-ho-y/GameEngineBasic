// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyPoolManager.generated.h"

class AEnemyBase;


// EnemyPool이라는 이름의 구조체를 만들어줌
// 이 구조체를 통해 풀링된 적들을 저장, 관리 할거임
USTRUCT(BlueprintType)
struct FEnemyPool
{
	GENERATED_BODY()
	UPROPERTY()
	TArray<AEnemyBase*> PooledEnemies;
};

USTRUCT(BlueprintType)
struct FEnemySpawnProfile // 나중에 무한스테이지를 만들 때에 적 가중치를 위한 SpawnProfile
{
	GENERATED_BODY()
	
	// 스폰할 적의 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBase> EnemyClass;

	// 체력, 공격력 배수 등 추가 정보
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// float HealthMultiplier = 1.0f;	
};
UCLASS()
class GAMEENGINEBASIC_API AEnemyPoolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEnemyPoolManager();

	AEnemyBase* SpawnEnemy(const FEnemySpawnProfile& Profile, const FVector& Location, const FRotator& Rotation);
	void ReturnEnemy(AEnemyBase* Enemy);

	UFUNCTION(BlueprintCallable, Category = "Enemy Pool")
	void SpawnEnemiesAtSpawnPoints();
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pool Settings")
	TMap<TSubclassOf<AEnemyBase>, FEnemyPool> EnemyPoolMap;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pool Settings")
	TMap<TSubclassOf<AEnemyBase>, int32> EnemyPoolSizes;
	
};
