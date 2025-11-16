#include "EnemyDroneAnimInstance.h"

#include "DroneMovementComponent.h"
#include "KismetAnimationLibrary.h"

class UDroneMovementComponent;

void UEnemyDroneAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!OwnerPawn)
		OwnerPawn = TryGetPawnOwner();
	if (!OwnerPawn) return;

	UDroneMovementComponent* MoveComp =
		OwnerPawn->FindComponentByClass<UDroneMovementComponent>();

	if (!MoveComp) return;

	FVector Velocity = MoveComp->Velocity;

	FVector VelDir = Velocity.GetSafeNormal();
	FVector LocalDir = OwnerPawn->GetActorTransform().InverseTransformVector(VelDir);

	MoveDirForward = LocalDir.X;
	MoveDirRight = LocalDir.Y;
}