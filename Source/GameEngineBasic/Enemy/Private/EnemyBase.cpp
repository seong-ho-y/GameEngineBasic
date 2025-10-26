// Fill out your copyright notice in the Description page of Project Settings.


#include "../Public/EnemyBase.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameEngineBasic/Components/public/Attack.h"
#include "GameEngineBasic/Components/public/Damageable.h"
#include "GameEngineBasic/Components/public/HealthComp.h"
#include "GameEngineBasic/Components/public/ShooterComp.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/Pawn.h"

class AAIController;
// Sets default values
AEnemyBase::AEnemyBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// 1. 루트 컴포넌트가 될 콜라이더를 생성하고 루트로 지정
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	RootComponent = CollisionComponent;

	// 2. 메시 컴포넌트를 생성하고 루트에 부착
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	// 3. 이동 컴포넌트를 생성
	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	HealthComp = CreateDefaultSubobject<UHealthComp>("HealthComp");
	ShooterComp = CreateDefaultSubobject<UShooterComp>("ShooterComp");

	MaxHealth = 1.0f;
	CurrentHealth = MaxHealth;
}

// Called when the game starts or when spawned
void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AICache) AICache = Cast<AAIController>(GetController());
	if (!BlackboardCache && AICache) BlackboardCache = AICache->GetBlackboardComponent();

	if (!BlackboardCache)
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s: No Blackboard yet"), *GetName());
		return;
	}

	const float ThrottleValue = BlackboardCache->GetValueAsFloat(TEXT("ThrottleValue"));
	UE_LOG(LogTemp, Verbose, TEXT("%s Throttle=%.2f"), *GetName(), ThrottleValue);
	if (!BlackboardCache && AICache)
	{
		BlackboardCache = AICache->GetBlackboardComponent();
	}
	if (!BlackboardCache)
	{
		return;
	}
	

	// MovementComp 찾아오기 가져오기
	if (UFloatingPawnMovement* FloatingMovement = Cast<UFloatingPawnMovement>(GetMovementComponent()))
	{
		// --- 부드러운 추진감 계산용 변수들 ---
		static FVector CurrentVelocity = FVector::ZeroVector;   // 현재 속도
		const FVector Forward = GetActorForwardVector();         // 현재 진행 방향
		const float TargetSpeed = MaxSpeed * ThrottleValue;      // 목표 속도
		const float Accel = BaseAcceleration;                    // 가속률 (기본값)

		// 속도를 부드럽게 보간 (가속/감속 모두 처리)
		const float InterpSpeed = (ThrottleValue > 0.f) ? Accel : Deceleration;
		const FVector DesiredVelocity = Forward * TargetSpeed;

		// 부드럽게 속도 보간 (관성 효과)
		CurrentVelocity = FMath::VInterpTo(CurrentVelocity, DesiredVelocity, DeltaTime, InterpSpeed);

		// 실제 이동 입력으로 전달
		const FVector MoveDirection = CurrentVelocity.GetSafeNormal();
		const float MoveScale = CurrentVelocity.Size() / MaxSpeed;

		AddMovementInput(MoveDirection, MoveScale);
	}
}

// 이 부분이 AI와 Pawn을 연결하는 핵심입니다.
void AEnemyBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 이 Pawn을 조종하는 컨트롤러가 AIController인지 확인하고 캐시에 저장
	AICache = Cast<AAIController>(NewController);
	if (AICache)
	{
		// AIController가 사용하는 블랙보드를 가져와 캐시에 저장
		BlackboardCache = AICache->GetBlackboardComponent();
		UE_LOG(LogTemp, Warning, TEXT("AICache is valid"));
		if (!BlackboardCache)
		{
			UE_LOG(LogTemp, Error, TEXT("BB is not found"));
		}
	}
}

// Called to bind functionality to input
void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyBase::ApplyDamage_Implementation(float DamageAmount, AController* InstigatorController, AActor* DamageCauser,
	FVector HitLoc, TSubclassOf<UDamageType> DamageType)
{
	SpawnVfx(DamagedVfx);
	CurrentHealth -= DamageAmount;
	UE_LOG(LogTemp, Log, TEXT("%s took %.1f damage (HP: %.1f)"), *GetName(), DamageAmount, CurrentHealth);

	if (CurrentHealth <= 0.f)
	{
		Die_Implementation(DamageCauser);
	}
	
}
float AEnemyBase::TakeDamage(
	float DamageAmount,
	FDamageEvent const& DamageEvent,
	AController* EventInstigator,
	AActor* DamageCauser)
{
	Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	// 여기서 실제 데미지 처리
	ApplyDamage(DamageAmount, EventInstigator, DamageCauser, GetActorLocation(), DamageEvent.DamageTypeClass);

	return DamageAmount;
}

void AEnemyBase::Die_Implementation(AActor* Killer)
{
	UE_LOG(LogTemp, Warning, TEXT("%s Died"), *GetName());
	SpawnVfx(ExplosionVfx);

	// AI 정지
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (AIController->GetBrainComponent())
		{
			AIController->GetBrainComponent()->StopLogic("Died");
		}
	}

	// 풀에 반환 (DeActivate)
	DeActivate();
}

bool AEnemyBase::IsDead_Implementation() const
{
	return IDamageable::IsDead_Implementation();
}

void AEnemyBase::Attack_Implementation()
{
	IAttack::Attack_Implementation();
}

void AEnemyBase::Activate()
{
	UE_LOG(LogTemp, Warning, TEXT("%s Activated"), *GetName());

	// 1. 액터를 보이게 하고, 충돌 및 틱을 활성화
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	
	// 체력 리셋 (임시)
	CurrentHealth = MaxHealth;


	
	// 2. 컴포넌트의 상태를 초기화합니다.
	// HealthComp에 체력과 쉴드를 최대로 설정해주기
	// Descriptor를 써서 할거같긴함
	if (HealthComp)
	{
		HealthComp->InitStats();
	}

	// ShooterComp 등 다른 컴포넌트
	// if (ShooterComp)
	// {
	//     ShooterComp->ResetAmmo(); 
	// }

	// 3. AI 로직을 다시 시작
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBrainComponent())
	{
		AIController->GetBrainComponent()->StartLogic();
	}
    
	/*
	// 4. 캐릭터 이동을 활성화
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		GetCharacterMovement()->Activate();
	}
	*/
}
void AEnemyBase::DeActivate()
{
	UE_LOG(LogTemp, Warning, TEXT("%s Deactivated"), *GetName());

	// 1. 액터를 숨기고, 충돌 및 틱을 비활성화하여 성능 부하를 줄입니다.
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);

	// 2. AI 로직을 정지시킵니다.
	AAIController* AIController = Cast<AAIController>(GetController());
	if (AIController && AIController->GetBrainComponent())
	{
		AIController->GetBrainComponent()->StopLogic("Deactivated by Object Pool");
	}

	/*
	// 3. 캐릭터의 움직임을 즉시 멈추고 비활성화합니다.
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
	}
	*/
}

void AEnemyBase::SpawnVfx(UNiagaraSystem* Vfx)
{
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Vfx, GetActorLocation());
}