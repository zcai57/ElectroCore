// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Dodge.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectRemoved.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UGA_Dodge::UGA_Dodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag("Input.Action.Dodge"));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dodge"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Input.Action.LightCombo"));
	//CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
}

void UGA_Dodge::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	// Cancel attack ability
	ASC->CancelAbilities(&CancelAbilitiesWithTag); // cancels all abilities owning those tags

	if (ASC && DodgeWindowEffectClass)
	{
		FGameplayEffectContextHandle Ctx = ASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecH = ASC->MakeOutgoingSpec(DodgeWindowEffectClass, 1.f, Ctx);
		// Apply DodgeEffect (can dodge attack, now moved to counter success)
		// if (SpecH.IsValid())
		// {
		// 	// Option A: drive duration directly in code
		// 	SpecH.Data->SetDuration(DodgeDuration, /*bLockDuration*/ false);
		//
		// 	DodgeGEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecH.Data.Get());
		// }
		auto* WaitRemoved = UAbilityTask_WaitGameplayEffectRemoved::WaitForGameplayEffectRemoved(this, DodgeGEHandle);
		WaitRemoved->OnRemoved.AddDynamic(this, &UGA_Dodge::OnDodgeGE_Removed);
		WaitRemoved->ReadyForActivation();
	}

	GetDodgeDirection();
	ChooseMontageAndPlay();
	ExecuteGameplayCue();
	LaunchCharacter();
}

void UGA_Dodge::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{

	// Remove Game Effect
	/*UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (DodgeGEHandle.IsValid() && ASC)
	{
		ASC->RemoveActiveGameplayEffect(DodgeGEHandle);
		DodgeGEHandle.Invalidate();
	}*/

	DodgeDir = FName("B");
	DodgeDirWS = FVector::ZeroVector;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Dodge::GetDodgeDirection()
{
	if (!CurrentActorInfo && !CurrentActorInfo->AvatarActor.Get()) return;

	ACharacter* Char = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!Char) return;

	FVector Input = Char->GetLastMovementInputVector(); // already normalized-ish
	Input.Z = 0.f;

	// Deal with no movement input
	if (Input.IsNearlyZero())
	{
		Input = -Char->GetActorForwardVector();
		Input.Z = 0.f;
	}
	Input.Normalize();

	// 2) Resolve against CAMERA yaw (so “W” = forward relative to camera)
	const FRotator ControlYaw(0.f,
		Char->GetControlRotation().Yaw, 0.f);

	// To local (camera) space
	const FVector Local = ControlYaw.UnrotateVector(Input);
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Local.Y, Local.X)); // -180..180
		
	// 3) Pick one of 8 sectors (45° each)
	auto PickSection8 = [](float Deg)->FName
		{
			if (Deg > -22.5f && Deg <= 22.5f)   return FName("F");
			else if (Deg > 22.5f && Deg <= 67.5f)   return FName("FR");
			else if (Deg > 67.5f && Deg <= 112.5f)   return FName("R");
			else if (Deg > 112.5f && Deg <= 157.5f)   return FName("BR");
			else if (Deg <= -157.5f || Deg > 157.5f)   return FName("B");
			else if (Deg > -157.5f && Deg <= -112.5f)  return FName("BL");
			else if (Deg > -112.5f && Deg <= -67.5f)  return FName("L");
			else /* -67.5 .. -22.5 */                  return FName("FL");
		};

	DodgeDir = PickSection8(AngleDeg);

	const TMap<FName, FVector> DirLUT = {
		{ "F",  FVector(1,  0, 0) }, { "FR", FVector(1,  1, 0).GetSafeNormal() },
		{ "R",  FVector(0,  1, 0) }, { "BR", FVector(-1,  1, 0).GetSafeNormal() },
		{ "B",  FVector(-1,  0, 0) }, { "BL", FVector(-1, -1, 0).GetSafeNormal() },
		{ "L",  FVector(0, -1, 0) }, { "FL", FVector(1, -1, 0).GetSafeNormal() },
	};
	const FVector* DirLocal = DirLUT.Find(DodgeDir);
	// if null, default to forward
	const FVector LocalDir = DirLocal ? *DirLocal : FVector(1, 0, 0);

	DodgeDirWS = ControlYaw.RotateVector(LocalDir);
	DodgeDirWS.Normalize();

	if (bEnableDebug)
	{
		const FVector Start = Char->GetActorLocation();
		const FVector End = Start + DodgeDirWS * 200.0f; // length = 200 units

		DrawDebugLine(
			Char->GetWorld(),
			Start,
			End,
			FColor::Cyan,
			false,
			2.0f,
			0,
			4.0f
		);
	}
}

void UGA_Dodge::ChooseMontageAndPlay()
{
	// Cancel Current Active Montage
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	if (ASC)
	{
		ASC->CurrentMontageStop(0.1f);
	}

	// Choose Montage to Play
	UAnimMontage* MontageToPlay = DodgeB;
	if (DodgeDir == FName("F"))
	{
		MontageToPlay = DodgeF;
	}
	else if (DodgeDir == FName("B"))
	{
		MontageToPlay = DodgeB;
	}
	else if (DodgeDir == FName("L"))
	{
		MontageToPlay = DodgeL;
	}
	else if (DodgeDir == FName("R"))
	{
		MontageToPlay = DodgeR;
	}
	else if (DodgeDir == FName("FL"))
	{
		MontageToPlay = DodgeFL;
	}
	else if (DodgeDir == FName("FR"))
	{
		MontageToPlay = DodgeFR;
	}
	else if (DodgeDir == FName("BL"))
	{
		MontageToPlay = DodgeBL;
	}
	else if (DodgeDir == FName("BR"))
	{
		MontageToPlay = DodgeBR;
	}
	// Active MontageTask
	auto ActiveTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay, 1.0f, NAME_None, /*bStopWhenAbilityEnds*/ true, 1.0f);
	ActiveTask->ReadyForActivation();

	ActiveTask->OnBlendOut.AddDynamic(this, &UGA_Dodge::EndDodge);
	ActiveTask->OnCompleted.AddDynamic(this, &UGA_Dodge::EndDodge);
	ActiveTask->OnInterrupted.AddDynamic(this, &UGA_Dodge::EndDodge);
	ActiveTask->OnCancelled.AddDynamic(this, &UGA_Dodge::EndDodge);
}

void UGA_Dodge::EndDodge()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


void UGA_Dodge::LaunchCharacter()
{
	AActor* CurActor = CurrentActorInfo->AvatarActor.Get();
	if (!CurActor) return;

	ACharacter* CurChar = Cast<ACharacter>(CurActor);
	if (CurChar)
	{
		DodgeDirWS.Z = 1;
		FVector Vel = DodgeDirWS * FVector(XYstrength, XYstrength, Zstrength);
		CurChar->LaunchCharacter(Vel, true, true);
	}

}

void UGA_Dodge::ExecuteGameplayCue()
{
	AActor* Self = GetActorInfo().AvatarActor.Get();
	if (!Self) return;

	FVector ActorLoc = Self->GetActorLocation();

	// Trace downward to find ground Z
	FCollisionQueryParams ColParams;
	ColParams.AddIgnoredActor(Self);
	const FVector Start = ActorLoc + FVector(0.f, 0.f, 50.f);
	const FVector End = ActorLoc - FVector(0.f, 0.f, 500.f);

	/*FHitResult Hit;
	float GroundZ = ActorLoc.Z;
	if (Self->GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, ColParams))
	{
		GroundZ = Hit.Location.Z;
	}*/

	// Execute LightAtkHit GameplayCue
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	FGameplayCueParameters Params;
	Params.Instigator = Self;
	//Params.Location = FVector(ActorLoc.X, ActorLoc.Y, GroundZ);
	Params.Location = ActorLoc;
	Params.Normal = DodgeDirWS;
	
	ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.Dodge"), Params);
}

/// <summary>
/// This is a failsafe function that will make sure Dodge ability end
/// Delete later ! Obsolete
/// </summary>
void UGA_Dodge::OnDodgeGE_Removed(const FGameplayEffectRemovalInfo& RemovalInfo)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
