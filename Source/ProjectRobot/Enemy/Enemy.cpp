// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "AbilitySystemComponent.h"
#include "EnemyControllerBase.h"
#include "ProjectRobot/Weapon/WeaponBase.h"
#include "ProjectRobot/AttributeSet/StartingAttributeSet.h"
#include "ProjectRobot/GAS/RobotAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProjectRobot/ActorComponents/AttackComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set collision for attacks
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	WeaponChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponChild"));
	WeaponChild->SetupAttachment(GetMesh(), FName("RightHandSocket"));

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = RotationRate; // degrees per second
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;
	
	// Setup GAS
	AbilitySystemComponent = CreateDefaultSubobject<URobotAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// Motion Warping
	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));

	// Attack Component
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent1"));
}

void AEnemy::SetStrafingMovement(bool bEnable)
{
	bIsStrafing = bEnable;

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp) return;

	if (bEnable)
	{
		// Strafing mode: face focus/target instead of move direction
		MoveComp->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;

		UE_LOG(LogTemp, Log, TEXT("%s entered strafing movement mode."), *GetName());
	}
	else
	{
		// Normal locomotion
		MoveComp->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;

		UE_LOG(LogTemp, Log, TEXT("%s returned to normal movement mode."), *GetName());
	}
}


// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	EnemyController = Cast<AEnemyControllerBase>(Controller);
	// Set up Attribute
	if (IsValid(AbilitySystemComponent))
	{
		StartAttributeSet = AbilitySystemComponent->GetSet<UStartingAttributeSet>();
	}

	// Set up weapon
	AWeaponBase* Weapon = Cast<AWeaponBase>(WeaponChild->GetChildActor());
	if (Weapon)
	{
		Weapon->SetOwner(this);
		Weapon->BindToOwnerAttackComponent(AttackComponent);
	}

	AddCharacterAbilities();
}

void AEnemy::AddCharacterAbilities()
{
	URobotAbilitySystemComponent* ASC = CastChecked<URobotAbilitySystemComponent>(AbilitySystemComponent);

	if (!HasAuthority()) return;

	ASC->InitAbilityActorInfo(this, this);
	ASC->AddCharacterAbilities(StartupAbilities);

	// Register Tag events
	ASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("State.Immobile"), EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &AEnemy::OnImmobileTagChanged);
}

void AEnemy::OnImmobileTagChanged(FGameplayTag, int32 NewCount)
{
	
	 if (NewCount > 0)
	 {
		bImmobile = true;
	 	if (EnemyController) EnemyController->StopMovement();
	 }
	 else
	 {
	 	// Tag was removed: re-enable movement
		bImmobile = false;
	 }
}

void AEnemy::DrawDebugDirection()
{
	check(bDrawDebugDirection);
	if (!bDrawDebugDirection) return;
	if (!GetWorld()) return;

	const FVector ActorLoc = GetActorLocation();

	// Pawn (actor) facing direction — green
	const FVector PawnDir = GetActorForwardVector();
	DrawDebugLine(GetWorld(),
		ActorLoc,
		ActorLoc + PawnDir * 200.f,
		FColor::Green,   // Pawn facing = green
		false, 0.f, 0, 4.f);

	// Controller facing direction — red
	if (AController* Ctrl = GetController())
	{
		const FVector CtrlDir = Ctrl->GetControlRotation().Vector();
		DrawDebugLine(GetWorld(),
			ActorLoc,
			ActorLoc + CtrlDir * 200.f,
			FColor::Red,   // Controller facing = red
			false, 0.f, 0, 4.f);
	}
}


void AEnemy::UpdateWalkSpeed(float NewSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
}

void AEnemy::ClampMotionWarpDist(float dist)
{
	if (!MotionWarpingComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("MotionWarpingComp is null on %s"), *GetName());
		return;
	}

	FVector Target = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	FVector SelfLoc = GetActorLocation();
	AActor* FocusedActor = Cast<AEnemyControllerBase>(Controller)->CombatTarget;
	
	if (FocusedActor)
	{
		FVector ActorLoc = FocusedActor->GetActorLocation();
		Target = FocusedActor->GetActorLocation();

		Direction = ActorLoc - SelfLoc;
		float Distance = Direction.Size();

		if (Distance > dist)
		{
			Direction.Normalize();
			Target = dist * Direction + SelfLoc;
		}
	}
	else {
		// Use controller yaw when no target
		const float Yaw = Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw;
		Direction = FRotator(0.f, Yaw, 0.f).Vector();

		Target = SelfLoc + Direction * dist;
	}

	if (bDrawDebugAttack)
	{
		DrawDebugSphere(GetWorld(), Target, 3.0f, 12, FColor::Red, true, 5);	
	}

	if (Target != FVector::ZeroVector)
	{
		MotionWarpingComp->AddOrUpdateWarpTargetFromLocation("AttackTarget", Target);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("MotionWarp Target not Valid"));
	}
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bDrawDebugDirection)
	{
		DrawDebugDirection();
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

bool AEnemy::IsImmobile()
{
	return bImmobile;
}

bool AEnemy::IsStrafing()
{
	return bIsStrafing;
}

//FVector AEnemy::GetDesiredAttackAngle() const
//{
//	return FVector::ZeroVector;
//}

