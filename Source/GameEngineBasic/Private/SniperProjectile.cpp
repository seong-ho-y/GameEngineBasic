// Fill out your copyright notice in the Description page of Project Settings.


#include "SniperProjectile.h"

#include "SniperProjectile.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SphereComponent.h"

ASniperProjectile::ASniperProjectile()
{
	// 충돌 바인딩은 BeginPlay에서 처리할 수도 있음
}

void ASniperProjectile::BeginPlay()
{
	Super::BeginPlay();
	DamageAmount = 40.f;

	if (CollisionComp)
	{
		// 1) 부모 바인딩 제거
		CollisionComp->OnComponentHit.RemoveDynamic(this, &AProjectile::OnHit);

		// 2) 자식 바인딩 등록
		CollisionComp->OnComponentHit.AddDynamic(this, &ASniperProjectile::HandleSniperHit);
	}
}

void ASniperProjectile::HandleSniperHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!OtherActor) return;

	// 1) 넉백
	if (ACharacter* HitCharacter = Cast<ACharacter>(OtherActor))
	{
		FVector Dir = GetVelocity().GetSafeNormal();
		FVector Force = Dir * KnockbackStrength;
		Force.Z += KnockbackUpStrength;

		HitCharacter->LaunchCharacter(Force, true, true);
	}

	// 2) 원래 Projectile 데미지/Destroy/이펙트 실행
	// -> 부모 OnHit_Implementation 직접 호출
	OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

	// 디버그 메세지
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("LaunchCharacter"));
}
