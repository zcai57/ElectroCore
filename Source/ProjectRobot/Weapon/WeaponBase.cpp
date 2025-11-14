// Fill out your copyright notice in the Description page of Project Settings.
#include "WeaponBase.h"

#include "Components/BoxComponent.h"
#include "ProjectRobot/ActorComponents/AttackComponent.h"
#include "Components/SphereComponent.h"


// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	AttackHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackHitbox"));
	AttackHitbox->SetupAttachment(WeaponMesh);
	AttackHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackHitbox->SetCollisionObjectType(ECC_GameTraceChannel1); // Attack_Blockable
	AttackHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	AttackHitbox->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Overlap); // Counter Trace
}

UBoxComponent* AWeaponBase::GetHitBox()
{
	return AttackHitbox;
}

void AWeaponBase::DebugDraw(bool on)
{
	bDebugHitbox = on;
}

void AWeaponBase::BindToOwnerAttackComponent(UAttackComponent* component)
{
	if (component)
	{
		component->BindWeapon(this);
	} else
	{
		UE_LOG(LogTemp, Warning, TEXT("AttackComponent is invalid"));
	}
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
}

void AWeaponBase::DrawHitBox()
{
	if (!AttackHitbox) return;

	const FVector Center = AttackHitbox->GetComponentLocation();
	const FVector Extent = AttackHitbox->GetScaledBoxExtent();
	const FQuat Rotation = AttackHitbox->GetComponentQuat();

	DrawDebugBox(
		GetWorld(),
		Center,
		Extent,
		Rotation,   
		FColor::Red,
		false,
		0.f,
		0,
		1.f
	);
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bDebugHitbox) DrawHitBox();
}

