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

	// Setup Owner
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
	
}

/// DamageTrace for different AttackDeliveryTypes
/// @param Key 
/// @param AttackData 
void UAttackComponent::StartDamageTrace(FName Key,  UAttackTraceData* AttackData)
{
	PrimaryComponentTick.bCanEverTick = true;
	HitActors.Empty();

	currData = AttackData;
	
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

/// Start Counter(Parry) Trace Duration
/// @param TraceData 
void UAttackComponent::StartCounterTrace(const FTraceWindow& TraceData)
{
	ActiveCounterTraceData = TraceData;
	bCounterTraceActive = true;
}

/// End Damage Trace Duration
/// @param TraceData 
void UAttackComponent::EndDamageTrace(UAttackTraceData* AttackData)
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

/// End Counter(Parry) Trace Duration
/// @param TraceData 
void UAttackComponent::EndCounterTrace()
{
	bCounterTraceActive = false;
}


/// Counter(Parry) BoxTrace
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
			// If already hit, return
			if (OwnerASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("Event.HitReact")))
			{
				EndCounterTrace();
				return;
			}
			
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

/// Enable Weapon Hitbox
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

/// Diable Melee Weapon Hitbox
void UAttackComponent::DisableWeaponHitbox()
{
	if (!boundWeapon || !boundWeapon->GetHitBox()) return;
	boundWeapon->DebugDraw(false);
	
	UBoxComponent* hitBox = boundWeapon->GetHitBox();
	hitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	hitBox->OnComponentBeginOverlap.RemoveAll(this);
}

/// Bind melee weapon to access weapon triggerBox
/// Usually set up in the actual Pawn (OwnedWeaponClass -> AttackComp BindWeapon)
/// @param Weapon 
void UAttackComponent::BindWeapon(AWeaponBase* Weapon)
{
	boundWeapon = Weapon;
}

/// Delegate target function for Weapon or Projectile TriggerBox overlap
/// @param OverlappedComponent 
/// @param OtherActor 
/// @param OtherComp 
/// @param OtherBodyIndex 
/// @param bFromSweep 
/// @param SweepResult 
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
		
		HandleHitReactionAndHitStop(Data);

		HandleHitEffect(Data);
		
		// Send event to abilities to apply ability-specific gameplay effects
		Data.EventTag = FGameplayTag::RequestGameplayTag("Event.Attack.DealDmg");
		
	}
}

/// Trigger HitReact Event and HitStop Event
/// @param Payload 
void UAttackComponent::HandleHitReactionAndHitStop(FGameplayEventData& Payload)
{
	const FHitResult* Hit = Payload.ContextHandle.GetHitResult();
	if (!Hit) return;

	AActor* Target = Hit->GetActor();
	AActor* Attacker = const_cast<AActor*>(Payload.Instigator.Get()); // AvatarActor usually

	if (!Target || !Attacker) return;

	// Set correct HitReactTag and HitStop eventMagnitude
	FGameplayTag HitReactTag = FGameplayTag::RequestGameplayTag("Event.HitReact");

	check(currData);
	float HitStopDuration = currData->HitStopDuration;
	float Magnitude = currData->KnockbackMagnitude;
	
	if (currData)
	{
		switch (currData->AttackMagnitude)
		{
		case EAttackMagnitude::Heavy:
			HitReactTag = FGameplayTag::RequestGameplayTag("Event.HeavyHitReact");
			break;
		case EAttackMagnitude::Light:
			HitReactTag = FGameplayTag::RequestGameplayTag("Event.LightHitReact");
			break;
		}
	}

	// Send HitReact
	FGameplayEventData ReactPayload = Payload;
	ReactPayload.EventTag = HitReactTag;
	ReactPayload.EventMagnitude = Magnitude;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, ReactPayload.EventTag, ReactPayload);

	// Send HitStop to both target and attacker
	FGameplayTag HitStopTag = FGameplayTag::RequestGameplayTag("Event.HitStop");
	Payload.EventMagnitude = HitStopDuration;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, HitStopTag, Payload);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Attacker, HitStopTag, Payload);
}

/// Handle GameplayEffect after hit like Damage
/// @param Payload 
void UAttackComponent::HandleHitEffect(FGameplayEventData& Payload)
{
	check(currData);
	check(currOwner);
	check(Payload.Target);
	
	UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(currOwner);
	UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Payload.Target);
	
	UE_LOG(LogTemp, Warning, TEXT("Attacker ASC: %p, Target ASC: %p"), ASC, TargetASC);
	UE_LOG(LogTemp, Warning, TEXT("Attacker: %s, Target: %s"), *GetNameSafe(currOwner), *GetNameSafe(Payload.Target));
	if (currData && currData->DamageEffect)
	{
		// Damage Target
		if (currData->DamageEffect)
		{
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(currData->DamageEffect, 1.0f, TargetASC->MakeEffectContext());
			Spec.Data->SetSetByCallerMagnitude(currData->EnergyTag, currData->DamageToEnergy);
			Spec.Data->SetSetByCallerMagnitude(currData->HeatTag, currData->DamageToHeat);

			ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, TargetASC);
		}

		// Buff self
		if (currData->BuffEffect)
		{
			FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(currData->BuffEffect, 1.0f, ASC->MakeEffectContext());
			Spec.Data->SetSetByCallerMagnitude(currData->EnergyTag, currData->BuffToEnergy);
			Spec.Data->SetSetByCallerMagnitude(currData->HeatTag, currData->BuffToHeat); // No buff to heat

			ASC->ApplyGameplayEffectSpecToTarget(*Spec.Data, ASC);
		}
	}
}

/// Send Hit GameplayCue for sound/vfx
/// @param TargetActor 
/// @param TargetComp 
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

	FGameplayTag GameplayCue =	FGameplayTag::RequestGameplayTag("GameplayCue.LightAttackHit");
	if (currData)
	{
		switch (currData->AttackMagnitude)
		{
		case EAttackMagnitude::Heavy:
			GameplayCue = FGameplayTag::RequestGameplayTag("GameplayCue.HeavyAttackHit");
			break;
		case EAttackMagnitude::Light:
			GameplayCue = FGameplayTag::RequestGameplayTag("GameplayCue.LightAttackHit");
			break;
		}
	}
    // Execute cue
    ASC->ExecuteGameplayCue(GameplayCue, Params);
}

/// Helper function that compute ImpactPoint and ImpactNormal from OverlappedResult
/// @param ImpactPoint 
/// @param ImpactNormal 
/// @param TargetActor 
/// @param TargetComp 
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

/// Simple helper that Ignore Self
/// @return 
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
