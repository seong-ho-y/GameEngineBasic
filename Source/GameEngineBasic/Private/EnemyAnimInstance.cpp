// EnemyAnimInstance.cpp
#include "EnemyAnimInstance.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UEnemyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	auto Owner = Cast<ACharacter>(TryGetPawnOwner());
	if (!Owner) return;

	Speed = Owner->GetVelocity().Size();
	bIsInAir = Owner->GetCharacterMovement()->IsFalling();
}
