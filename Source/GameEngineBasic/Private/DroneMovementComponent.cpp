#include "DroneMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

UDroneMovementComponent::UDroneMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UDroneMovementComponent::TickComponent(
    float DeltaTime, f
    enum ELevelTick TickType, 
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!PawnOwner) return;

    MaintainHover(DeltaTime);
    ApplyDamping(DeltaTime);
}

/* ================= Hover ================= */
void UDroneMovementComponent::MaintainHover(float DeltaTime)
{
    FVector Loc = PawnOwner->GetActorLocation();
    FHitResult Hit;

    FVector Start = Loc;
    FVector End = Loc - FVector(0, 0, HoverTraceDistance);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(PawnOwner);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, Start, End, ECC_Visibility, Params);

    float TargetZ = Loc.Z;

    if (bHit)
        TargetZ = Hit.Location.Z + HoverHeight;

    float NewZ = FMath::FInterpTo(Loc.Z, TargetZ, DeltaTime, HoverInterpSpeed);

    PawnOwner->SetActorLocation(FVector(Loc.X, Loc.Y, NewZ));
}

/* ================= Movement ================= */
void UDroneMovementComponent::MoveInDirection(const FVector& WorldDir)
{
    FVector Dir = WorldDir.GetSafeNormal2D();
    Velocity = Dir * MoveSpeed;
}

/* ================= Strafe ================= */
void UDroneMovementComponent::StrafeLeft()
{
    Velocity = -PawnOwner->GetActorRightVector() * StrafeSpeed;
}

void UDroneMovementComponent::StrafeRight()
{
    Velocity = PawnOwner->GetActorRightVector() * StrafeSpeed;
}

/* ================= Approach / Retreat ================= */
void UDroneMovementComponent::MoveToward(const FVector& Target)
{
    FVector Dir = (Target - PawnOwner->GetActorLocation()).GetSafeNormal2D();
    Velocity = Dir * MoveSpeed;
}

void UDroneMovementComponent::MoveAway(const FVector& Target)
{
    FVector Dir = (PawnOwner->GetActorLocation() - Target).GetSafeNormal2D();
    Velocity = Dir * MoveSpeed;
}

/* ================= Damp (부드럽게 멈춤) ================= */
void UDroneMovementComponent::ApplyDamping(float DeltaTime)
{
    Velocity = FMath::VInterpTo(Velocity, FVector::ZeroVector, DeltaTime, VelocityDamp);

    // Move pawn
    FVector NewLoc = PawnOwner->GetActorLocation() + Velocity * DeltaTime;
    PawnOwner->SetActorLocation(NewLoc, true);
}
