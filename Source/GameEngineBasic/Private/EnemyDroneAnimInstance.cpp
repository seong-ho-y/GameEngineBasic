#include "EnemyDroneAnimInstance.h"
#include "KismetAnimationLibrary.h"

void UEnemyDroneAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerPawn)
		OwnerPawn = TryGetPawnOwner();

	if (!OwnerPawn) return;

	FVector Velocity = OwnerPawn->GetVelocity();

	// Normalize (정규화)
	FVector VelDir = Velocity.GetSafeNormal2D();

	// 드론 Transform 기준의 로컬 이동 벡터
	FVector LocalDir = OwnerPawn->GetActorTransform().InverseTransformVector(VelDir);

	// LocalDir.X = Forward(+) / Back(-)
	// LocalDir.Y = Right(+) / Left(-)

	MoveDirForward = LocalDir.X;   // -1 ~ +1
	MoveDirRight   = LocalDir.Y;   // -1 ~ +1

	
}
