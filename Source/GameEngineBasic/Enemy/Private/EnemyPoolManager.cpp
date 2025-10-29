// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/EnemyPoolManager.h"

#include "GameEngineBasic/Enemy/Public/EnemyBase.h"

class AEnemyBase;
// Sets default values
AEnemyPoolManager::AEnemyPoolManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}
void AEnemyPoolManager::BeginPlay()
{
	Super::BeginPlay();

	// 게임 시작 시, 설정된 수만큼 각 종류의 적을 미리 생성(풀링)
	for (const auto& Pair : EnemyPoolSizes)
	{
		TSubclassOf<AEnemyBase> EnemyClass = Pair.Key;
		int32 PoolSize = Pair.Value;

		if (!EnemyClass) continue;

		// TMap에 해당 클래스의 풀을 새로 만들기
		FEnemyPool NewPool;
        
		for (int32 i = 0; i < PoolSize; ++i)
		{
			// 액터를 월드에 스폰
			if (AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyClass, FVector::ZeroVector, FRotator::ZeroRotator))
			{
				NewEnemy->DeActivate();        // 생성 직후 비활성화 상태
				NewPool.PooledEnemies.Add(NewEnemy); // 풀에 추가
			}
		}
        
		EnemyPoolMap.Add(EnemyClass, NewPool);
	}
}


AEnemyBase* AEnemyPoolManager::SpawnEnemy(const FEnemySpawnProfile& Profile, const FVector& Location, const FRotator& Rotation)
{
	if (!Profile.EnemyClass) return nullptr;
	FEnemyPool* Pool = EnemyPoolMap.Find(Profile.EnemyClass);
	if (!Pool)
	{
		UE_LOG(LogTemp, Error, TEXT("SpawnEnemy failed: No pool found for class %s."), *Profile.EnemyClass->GetName());
		return nullptr;
	}

	AEnemyBase* SpawnedEnemy = nullptr;

	// 1. 풀에 사용 가능한 적 확인
	if (Pool->PooledEnemies.Num() > 0)
	{
		// 풀에서 꺼내서 사용
		SpawnedEnemy = Pool->PooledEnemies.Pop();
	}
	// 2. 풀이 없으면
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Pool for %s is empty. Spawning a new one dynamically."), *Profile.EnemyClass->GetName());
		SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(Profile.EnemyClass);
	}
	
	if (SpawnedEnemy)
	{
		// 스폰 위치와 회전값을 설정합니다.
		SpawnedEnemy->SetActorLocationAndRotation(Location, Rotation);
		// 적을 활성화합니다.
		SpawnedEnemy->Activate();
		return SpawnedEnemy;
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to spawn enemy of class %s."), *Profile.EnemyClass->GetName());
	return nullptr;
}

void AEnemyPoolManager::ReturnEnemy(AEnemyBase* Enemy)
{
	if (!Enemy) return;

	// 반환된 적이 속한 클래스의 풀을 찾기
	if (FEnemyPool* Pool = EnemyPoolMap.Find(Enemy->GetClass()))
	{
		// 풀에 다시 추가
		Pool->PooledEnemies.Add(Enemy);
	}
	else
	{
		// 풀에 속하지 않은 적이라면 그냥 파괴
		UE_LOG(LogTemp, Warning, TEXT("Returned enemy of class %s has no corresponding pool. Destroying it."), *Enemy->GetClass()->GetName());
		Enemy->Destroy();
	}
}


// Called when the game starts or when spawned
