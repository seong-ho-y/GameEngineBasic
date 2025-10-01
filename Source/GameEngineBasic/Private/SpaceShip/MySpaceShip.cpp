
#include "SpaceShip/MySpaceShip.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/LocalPlayer.h"

// �⺻ ������
AMySpaceShip::AMySpaceShip()
{
	PrimaryActorTick.bCanEverTick = true;

	ShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMesh; // ��Ʈ ������Ʈ�� ����

	ShipMesh->SetSimulatePhysics(true);
	ShipMesh->BodyInstance.bEnableGravity = false;
	ShipMesh->SetEnableGravity(false);
	
	// ���� ���� (���� ������ ����)
	// ���� �������� �� ���� ����ϴ�. ���ּ�ó�� �̲������� ������ �ַ��� ���� ���� ����ϼ���.
	ShipMesh->SetLinearDamping(0.5f);
	
	// ���ӵ� ���� (ȸ�� ������ ����)
	// ���� �������� ȸ���� �� ���� ����ϴ�.
	ShipMesh->SetAngularDamping(4.0f);


	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = false;

	// --- �Ʒ� ī�޶� ���� �ɼ� Ȱ��ȭ ---
	SpringArm->bEnableCameraLag = true;         // ��ġ ���� Ȱ��ȭ
	SpringArm->CameraLagSpeed = 5.0f;           // ��ġ ���� �ӵ�

	SpringArm->bEnableCameraRotationLag = true; // ȸ�� ���� Ȱ��ȭ
	SpringArm->CameraRotationLagSpeed = 5.0f;   // ȸ�� ���� �ӵ�

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	
}

// ���� ���� �� ȣ��Ǵ� �Լ�
void AMySpaceShip::BeginPlay()
{
	Super::BeginPlay();

}

// �� �����Ӹ��� ȣ��Ǵ� �Լ�
void AMySpaceShip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 1. ���� ���ּ��� ȸ�� ���� �����ɴϴ�. (���� ������ ���� ���� ��)
	const FRotator CurrentRotation = GetActorRotation();

	// 2. ��ǥ ���� ������ ����մϴ�. (���콺 X �Է¿� ���)
	const float TargetRollAngle = CurrentLookInput.X * MaxRollAngle;

	// 3. ���� ���� �������� ��ǥ ������ �ε巴�� ����(Interpolation)�մϴ�.
	const float NewRollAngle = FMath::FInterpTo(CurrentRotation.Roll, TargetRollAngle, DeltaTime, RollSpeed);

	// 4. ���� ������ ����� Pitch, Yaw ���� �����ϰ�, Roll ���� ���� ����� ������ �����Ͽ� ���ο� ȸ�� ���� ����ϴ�.
	const FRotator NewRotation = FRotator(CurrentRotation.Pitch, CurrentRotation.Yaw, NewRollAngle);

	// 5. ���� ȸ�� ���� ���ּ��� �����մϴ�.
	SetActorRotation(NewRotation);
}

// �÷��̾� �Է� ó��
void AMySpaceShip::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInput->BindAction(IA_MoveForward, ETriggerEvent::Triggered, this, &AMySpaceShip::MoveForward);
		EnhancedInput->BindAction(IA_MoveUpDown, ETriggerEvent::Triggered, this, &AMySpaceShip::MoveUpDown);
		EnhancedInput->BindAction(IA_Boost, ETriggerEvent::Triggered, this, &AMySpaceShip::Boost);
		EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AMySpaceShip::Look);
	}

	else {
		UE_LOG(LogTemp, Warning, TEXT("EnhancedInputComponent not found!"));
	}
}

void AMySpaceShip::MoveForward(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();
	float CurrentThrust = ThrustForce;

	// �ν�Ʈ ���¶�� �߷��� ������ŵ�ϴ�.
	if (bIsBoosting)
	{
		CurrentThrust *= BoostMultiplier;
	}

	// ���ּ��� �� �������� ���� ���մϴ�.
	// AddForce�� ������ ����(bAccelChange)�� true�� �ϸ� ������ ������� ������ ���ӵ��� ���ϴ�.
	ShipMesh->AddForce(GetActorForwardVector() * InputValue * CurrentThrust, NAME_None, true);

}

void AMySpaceShip::MoveUpDown(const FInputActionValue& Value)
{
	float InputValue = Value.Get<float>();

	// ���ּ��� ���� �������� ���� ���մϴ�.
	ShipMesh->AddForce(GetActorUpVector() * InputValue * UpDownForce, NAME_None, true);

}

void AMySpaceShip::Look(const FInputActionValue& Value)
{
	// �Է� ���� FVector2D�� �о�ɴϴ�.
	CurrentLookInput = Value.Get<FVector2D>();

	// Pitch (���� ȸ��) ��ũ: ���콺 Y �Է� * ���ּ� ������ ����
	// ���콺 Y�� ���� ���� �ø��� -���̹Ƿ�, -�� ���� ������ �����ݴϴ�.
	FVector PitchTorque = GetActorRightVector() * -CurrentLookInput.Y * TurnTorque;

	// Yaw (�¿� ȸ��) ��ũ: ���콺 X �Է� * ���ּ� ���� ����
	FVector YawTorque = GetActorUpVector() * CurrentLookInput.X * TurnTorque;

	// �� ��ũ�� ���ļ� �����մϴ�.
	ShipMesh->AddTorqueInDegrees(PitchTorque + YawTorque, NAME_None, true);
}

void AMySpaceShip::Boost()
{
	bIsBoosting = !bIsBoosting;
}