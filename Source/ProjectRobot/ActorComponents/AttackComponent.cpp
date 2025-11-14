// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/BoxComponent.h"
#include "Engine/OverlapResult.h"
#include "ProjectRobot/Weapon/WeaponBase.h"
#include "ProjectRobot/Data/AttackTraceData.h"
#include "ProjectRobot/Data/TraceWindow.h"

// Sets default values for this component's properties
UAttackComponent::UAttackComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAttackComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	currOwner = GetOwner();

}



// Called every frame
void UAttackComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bCounterTraceActive)
	{
		DoCounterTrace();
	}
	
	// for (auto& It : TraceDataSet)
	// {
	// 	const FName& Key = It.Key;        
	// 	const & Value = It.Value;
	//
	// 	DoDamageTrace(Key, Value);
	// }
}

void UAttackComponent::StartDamageTrace(FName Key, const UAttackTraceData* AttackData)
{
	PrimaryComponentTick.bCanEverTick = true;
	HitActors.Empty();
	
	switch (AttackData->DeliveryType)
	{
	case EAttackDeliveryType::Melee:
		EnableWeaponHitbox();
		break;
	case EAttackDeliveryType::Projectile:
		break;
	case EAttackDeliveryType::AOE:
		break;
	}
}

void UAttackComponent::StartCounterTrace(const FTraceWindow& TraceData)
{
	ActiveCounterTraceData = TraceData;
	bCounterTraceActive = true;
}

void UAttackComponent::EndDamageTrace(const UAttackTraceData* AttackData)
{
	switch (AttackData->DeliveryType)
	{
	case EAttackDeliveryType::Melee:
		DisableWeaponHitbox();
		break;
	case EAttackDeliveryType::Projectile:
		break;
	case EAttackDeliveryType::AOE:
		break;
	}
}

void UAttackComponent::EndCounterTrace()
{
	bCounterTraceActive = false;
}

void UAttackComponent::DoCounterTrace()
{
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Shape = FCollisionShape::MakeBox(ActiveCounterTraceData.BoxHalfExtent);
	
	FVector Center = currOwner->GetActorLocation() + currOwner->GetActorForwardVector() * ActiveCounterTraceData.Range;
	
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Center,
		FQuat::Identity,
		ActiveCounterTraceData.TraceChannel, // "Counter"
		Shape
	);

	if (bEnableDebug)
	{
		DrawDebugBox(
		   GetWorld(),
		   Center,                              
		   ActiveCounterTraceData.BoxHalfExtent,    
		   FColor::Green,                       
		   false,                              
		   0.0f                                
		);
	}
	
	
	for (FOverlapResult& Hit : Overlaps)
	{
		AActor* HitActor = Hit.GetActor();
		UPrimitiveComponent* HitComp = Hit.GetComponent();
		
		if (!HitActor || !HitComp || HitActor == currOwner) continue;

		// Filter only Attack_Blockable object types
		if (HitComp->GetCollisionObjectType() != ECC_GameTraceChannel1) // 1 = Attack_Blockable
			continue;

		// Owner of the weapons that's overlapped
		AActor* TargetActor = HitActor->GetOwner();
		check(TargetActor);
		
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor);
		UAbilitySystemComponent* OwnerASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(currOwner);
		// Trigger target's CountReact
		if (ASC && OwnerASC)
		{
			
			// When player block, trigger reaction to block
			if (OwnerASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.Block")))
			{
				FGameplayEventData Data;
				// Tell target react to block
				Data.Target = TargetActor; 
				Data.Instigator = currOwner; 
				Data.EventTag = FGameplayTag::RequestGameplayTag("Event.Counter.React");
				Data.EventMagnitude = 1000.f;
				if (currOwner->HasAuthority())
				{
					UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(TargetActor, Data.EventTag, Data);
				}
				
				// Execute CounterSuccess GameplayCue
				FGameplayCueParameters Params;
				Params.Instigator = currOwner;
				Params.EffectCauser = currOwner;

				FVector HitLocation = FVector::ZeroVector;
				FVector HitNormal = FVector::ZeroVector;
				// Compute Impact
				// Computer impact
				ComputeImpact(HitLocation, HitNormal, TargetActor, HitComp);
				// Pass to Event data
				Params.Location = HitLocation;
				Params.Normal = HitNormal;

				OwnerASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.CounterSuccess"), Params);
			}

			// Tell owner counter success (dodge/block)
			FGameplayEventData OwnerData;
			OwnerData.Target = currOwner;
			OwnerData.Instigator = currOwner;
			OwnerData.EventTag = FGameplayTag::RequestGameplayTag("Event.Counter.Success");
			if (currOwner->HasAuthority())
			{
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(currOwner, OwnerData.EventTag, OwnerData);
			}

			// End The Trace since counter succeeded
			EndCounterTrace();
			return;
		}
		
	}
}

void UAttackComponent::EnableWeaponHitbox()
{
	if (!boundWeapon || !boundWeapon->GetHitBox()) return;
	if (bEnableDebug) boundWeapon->DebugDraw(true);
	
	UBoxComponent* hitBox = boundWeapon->GetHitBox();
	hitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (!hitBox->OnComponentBeginOverlap.IsAlreadyBound(this, &UAttackComponent::WeaponHitRegister))
	{
		hitBox->OnComponentBeginOverlap.AddDynamic(this, &UAttackComponent::WeaponHitRegister);
	}
}

void UAttackComponent::DisableWeaponHitbox()
{
	if (!boundWeapon || !boundWeapon->GetHitBox()) return;
	boundWeapon->DebugDraw(false);
	
	UBoxComponent* hitBox = boundWeapon->GetHitBox();
	hitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	hitBox->OnComponentBeginOverlap.RemoveAll(this);
}

void UAttackComponent::BindWeapon(AWeaponBase* Weapon)
{
	boundWeapon = Weapon;
}

void UAttackComponent::WeaponHitRegister(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == currOwner) return;
	if (HitActors.Contains(OtherActor)) return;
	HitActors.Add(OtherActor);

	SendHitGameplayCue(OtherActor, OtherComp);
	// GAS
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(currOwner))
	{
		FGameplayEventData Data;
		// Set target and self
		Data.Target = OtherActor;
		Data.Instigator = currOwner;
		// Add hitResult to EffectContext
		FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
		Context.AddHitResult(SweepResult, true);
		Data.ContextHandle = Context;

		Data.EventTag = FGameplayTag::RequestGameplayTag("Event.Attack.DealDmg");
		if (currOwner->HasAuthority())
		{
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(currOwner, Data.EventTag, Data);
		}

		
	}
}

void UAttackComponent::SendHitGameplayCue(
    AActor* TargetActor,
    UPrimitiveComponent* TargetComp)
{
    if (!currOwner || !TargetActor)
        return;

    // Get owner’s ASC
    UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(currOwner);
    if (!ASC)
        return;

    // Prepare cue parameters
    FGameplayCueParameters Params;
    Params.Instigator = currOwner;
    Params.EffectCauser = currOwner;
    Params.TargetAttachComponent = TargetActor->FindComponentByClass<USkeletalMeshComponent>();

    // Compute hit location + normal using hitbox geometry
    FVector ImpactPoint = FVector::ZeroVector;
    FVector ImpactNormal = FVector::ZeroVector;

	// Computer impact
    ComputeImpact(ImpactPoint, ImpactNormal, TargetActor, TargetComp);

    Params.Location = ImpactPoint;
    Params.Normal = ImpactNormal;

    // Optional debug draw
    if (bEnableDebug)
    {
        DrawDebugSphere(GetWorld(), ImpactPoint, 5.f, 12, FColor::Yellow, false, 1.f);
        DrawDebugLine(GetWorld(), ImpactPoint, ImpactPoint + ImpactNormal * 30.f, FColor::Cyan, false, 1.f, 0, 1.f);
    }

    // Execute cue
    ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.LightAttack"), Params);
}

void UAttackComponent::ComputeImpact(FVector& ImpactPoint, FVector& ImpactNormal, AActor* TargetActor, UPrimitiveComponent* TargetComp)
{
	UBoxComponent* WeaponHitbox = boundWeapon->GetHitBox();

	if (WeaponHitbox && TargetComp)
	{
		const FVector BoxCenter = WeaponHitbox->GetComponentLocation();
		const FQuat BoxRot = WeaponHitbox->GetComponentQuat();
		const FVector Extent = WeaponHitbox->GetScaledBoxExtent();

		// Local target position in hitbox space
		const FVector LocalTarget = BoxRot.UnrotateVector(TargetComp->GetComponentLocation() - BoxCenter);
		const FVector ClampedLocal = FVector(
			FMath::Clamp(LocalTarget.X, -Extent.X, Extent.X),
			FMath::Clamp(LocalTarget.Y, -Extent.Y, Extent.Y),
			FMath::Clamp(LocalTarget.Z, -Extent.Z, Extent.Z)
		);

		ImpactPoint = BoxCenter + BoxRot.RotateVector(ClampedLocal);
		ImpactNormal = (TargetComp->GetComponentLocation() - ImpactPoint).GetSafeNormal();
	}
	else
	{
		ImpactPoint = TargetActor->GetActorLocation();
		ImpactNormal = (ImpactPoint - currOwner->GetActorLocation()).GetSafeNormal();
	}
}

FCollisionQueryParams UAttackComponent::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	if (currOwner)
	{
		currOwner->GetAllChildActors(CharacterChildren);
	}
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(currOwner);

	return Params;
}
