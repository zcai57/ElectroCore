// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "EnemyControllerBase.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameplayEffect.h"
#include "ProjectRobot/Weapon/WeaponBase.h"
#include "ProjectRobot/AttributeSet/StartingAttributeSet.h"
#include "ProjectRobot/GAS/RobotAbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Materials/MaterialInstance.h"
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

	AddCharacterAbilities();

	// Use Gameplay Effect to init stats.
	// FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	// EffectContext.AddSourceObject(this);
	//
	// FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(StartingAttributeEffect, 1.f, EffectContext);
	// if (SpecHandle.IsValid())
	// {
	// 	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	// }
	// AbilitySystemComponent->InitStats(UStartingAttributeSet::StaticClass(), DT_StartingAttributes);
	// Set up Attribute
	if (IsValid(AbilitySystemComponent))
	{
		StartAttributeSet = AbilitySystemComponent->GetSet<UStartingAttributeSet>();
	}

	UE_LOG(LogTemp, Warning, TEXT("Using DT: %s"), *GetNameSafe(DT_StartingAttributes));

	if (StartAttributeSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] Heat=%f Energy=%f Status=%f"),
			*GetName(), StartAttributeSet->GetHeat(), StartAttributeSet->GetEnergy(), StartAttributeSet->GetStatus());
	}

	// Set up weapon
	AWeaponBase* Weapon = Cast<AWeaponBase>(WeaponChild->GetChildActor());
	if (Weapon)
	{
		Weapon->SetOwner(this);
		Weapon->BindToOwnerAttackComponent(AttackComponent);
	}
	// Bind events to attribute changes
	BindAttributeDelegate();
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

	ASC->GenericGameplayEventCallbacks.FindOrAdd(
		   FGameplayTag::RequestGameplayTag("Event.DamageTaken")
	   ).AddUObject(this, &AEnemy::OnDamageTaken);
}

void AEnemy::BindAttributeDelegate()
{
	URobotAbilitySystemComponent* ASC = CastChecked<URobotAbilitySystemComponent>(AbilitySystemComponent);
	const UStartingAttributeSet* AttrSet = AbilitySystemComponent->GetSet<UStartingAttributeSet>();
	if (!AttrSet) return;
	
	// FGameplayAttribute EnergyAttr = UStartingAttributeSet::GetEnergyAttribute();
	// ASC->GetGameplayAttributeValueChangeDelegate(EnergyAttr).AddUObject(this, &AEnemy::OnEnergyChanged);
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

void AEnemy::TriggerExecution()
{
	check(LastHitInstigator);
	
	// Create payload for execution
	FGameplayEventData EventData;
	EventData.Instigator = LastHitInstigator;   // Who will perform the execution
	EventData.Target = this;

	FGameplayTag ExecTag = FGameplayTag::RequestGameplayTag("Event.Execution");

	// Send event to attacker
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		const_cast<AActor*>(LastHitInstigator), // convert weak ptr → raw ptr
		ExecTag,
		EventData
	);
	// Send event to self
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		this, ExecTag, EventData);

	bIsBeingExecuted = true;
}

void AEnemy::TriggerNormalDeath()
{
	if (bIsBeingExecuted) return;

	if (AbilitySystemComponent)
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this,
			FGameplayTag::RequestGameplayTag("Event.Death"),
			FGameplayEventData()
		);
	}
}


void AEnemy::OnDamageTaken(const FGameplayEventData* Payload)
{
	LastHitInstigator = Payload->Instigator.Get();
	LastHitWasMelee = Payload->TargetTags.HasTag(FGameplayTag::RequestGameplayTag("Event.MeleeAttack"));
}


void AEnemy::StartDissolve()
{
	USkeletalMeshComponent* CurMesh = GetMesh();
	TotalDissolveMat.Empty();

	int32 MatCount = CurMesh->GetNumMaterials();

	for (int32 i = 0; i < MatCount; ++i)
	{
		// Replace
		CurMesh->SetMaterial(i, DissolveMat);

		// Create dynamic version
		UMaterialInstanceDynamic* Dyn = CurMesh->CreateDynamicMaterialInstance(i);
		if (Dyn)
		{
			TotalDissolveMat.Add(Dyn);
		}
	}

	// Start dissolve
	bDissolving = true;
	DissolveAmount = -1.f;
}

void AEnemy::UpdateDissolve(float DeltaTime)
{
	const float DissolveSpeed = 2.f / DespawnTimeAfterDeath;
	if (bDissolving && DissolveMat)
	{
		DissolveAmount = FMath::Clamp(DissolveAmount + DeltaTime * DissolveSpeed, -1.f, 1.f);

		for (auto* Mat : TotalDissolveMat)
		{
			Mat->SetScalarParameterValue("Dissolve", DissolveAmount);
		}

		if (DissolveAmount >= 1.f)
		{
			bDissolving = false;
		}
	}
}

void AEnemy::OnDeathStart()
{
	if (bIsDying) return;
	bIsDying = true;

	// Death Tag
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer DeathTags;
		DeathTags.AddTag(FGameplayTag::RequestGameplayTag("State.Death"));
	
		AbilitySystemComponent->AddLooseGameplayTags(DeathTags);
	}

	EnemyController->OnDeath();
	
	if (LastHitWasMelee && LastHitInstigator)
	{
		TriggerExecution();
		return;
	}
	
	TriggerNormalDeath();
}

void AEnemy::OnDeathEnd()
{
	// Set lifetime and dissolve death effect
	SetLifeSpan(DespawnTimeAfterDeath);
	StartDissolve();
	

	USkeletalMeshComponent* MeshComp = GetMesh();

	// Ragdoll
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionProfileName(FName("Ragdoll"));
	MeshComp->WakeAllRigidBodies();

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
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
	// Trigger on DeathEnd
	UpdateDissolve(DeltaTime);
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

