// Fill out your copyright notice in the Description page of Project Settings.
#include "GA_HitReact.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"


UGA_HitReact::UGA_HitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Event.HitReact")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.HitReact"));
	
	// Set up Trigger
	FAbilityTriggerData Trigger;
	HitReactEventTag = FGameplayTag::RequestGameplayTag(FName("Event.HitReact"));
	Trigger.TriggerTag = HitReactEventTag;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
	
	// Disable movement & cancel attacks
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Immobile"));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Input.Action.LightCombo"));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag("Input.Action"));

	bRetriggerInstancedAbility = true;
}

void UGA_HitReact::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) // pays costs, checks cooldown/block tags if set up
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}

	// Check Avatar and Montage
	ACharacter* Char = Cast<ACharacter>(CurrentActorInfo->AvatarActor.Get());
	if (!Char || !HitReactForward || !HitReactBack || !HitReactLeft || !HitReactRight)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	check(TriggerEventData);
	const FHitResult* hit = TriggerEventData->ContextHandle.GetHitResult();
	if (hit)
	{
		GetHitDirection(hit->ImpactPoint);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("GA_HitReact: HitResult is empty!"))
	}
	
	// Bind to MontageEnd Notify
	auto* EndComboTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag(FName("Event.MontageEnd")));
	EndComboTask->EventReceived.AddDynamic(this, &UGA_HitReact::OnMontageEnd);
	EndComboTask->ReadyForActivation();

	// Stop Current
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->CurrentMontageStop(0.05f); // tiny blend-out
	}

	if (ActiveTask)
	{
		ActiveTask->EndTask();        // ends the task (triggers OnInterrupted)
		ActiveTask = nullptr;
	}
	
	UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	if (ASC->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.CounterSuccess")))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; // skip reacting if parry window active
	}

	UAnimMontage* MontageToPlay = HitReactLeft;
	// Play animation
	if (UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance())
	{
		if (hitDirection == EHitDireciton::EHD_None || hitDirection == EHitDireciton::EHD_Left)
		{
			MontageToPlay = HitReactLeft;
			/*Char->PlayAnimMontage(HitReactLeft, 1.0f);*/
		}
		else if (hitDirection == EHitDireciton::EHD_Right)
		{
			MontageToPlay = HitReactRight;
			/*Char->PlayAnimMontage(HitReactRight, 1.0f);*/
		}
		else if (hitDirection == EHitDireciton::EHD_Forward)
		{
			MontageToPlay = HitReactForward;
			/*Char->PlayAnimMontage(HitReactForward, 1.0f);*/
		}
		else  if (hitDirection == EHitDireciton::EHD_Back)
		{
			MontageToPlay = HitReactBack;
			/*Char->PlayAnimMontage(HitReactBack, 1.0f);*/
		}
		
	}

	ActiveTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, MontageToPlay, 1.0f, NAME_None, /*bStopWhenAbilityEnds*/ true, 1.0f);
	ActiveTask->ReadyForActivation();

	ActiveTask->OnBlendOut.AddDynamic(this, &UGA_HitReact::EndHitReact);
	ActiveTask->OnCompleted.AddDynamic(this, &UGA_HitReact::EndHitReact);
	ActiveTask->OnInterrupted.AddDynamic(this, &UGA_HitReact::EndHitReact);
	ActiveTask->OnCancelled.AddDynamic(this, &UGA_HitReact::EndHitReact);


	// Push back
	const AActor* Attacker = Cast<AActor>(TriggerEventData->Instigator.Get());
	FVector Dir = (Char->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D();

	const FVector Knockback = Dir * XYPushStrength + FVector(0, 0, ZKick);
	Char->LaunchCharacter(Knockback, /*bXYOverride*/true, /*bZOverride*/true);
}

void UGA_HitReact::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ActiveTask = nullptr;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_HitReact::GetHitDirection(const FVector& hit)
{
	if (AActor* Actor = CurrentActorInfo->AvatarActor.Get())
	{
		Actor->GetActorLocation();

		// Get the direction from actor to hit location (2D plane only)
		FVector HitDirection = (hit - Actor->GetActorLocation()).GetSafeNormal2D();

		// Get actor's forward and right vectors
		FVector ActorForward = Actor->GetActorForwardVector();
		FVector ActorRight = Actor->GetActorRightVector();

		// Calculate dot products
		float ForwardDot = FVector::DotProduct(ActorForward, HitDirection);
		float RightDot = FVector::DotProduct(ActorRight, HitDirection);

		// Determine which direction has stronger influence
		if (FMath::Abs(ForwardDot) > FMath::Abs(RightDot))
		{
			// Hit is more front/back
			hitDirection = (ForwardDot > 0.0f) ? EHitDireciton::EHD_Forward : EHitDireciton::EHD_Back;
		}
		else
		{
			// Hit is more left/right
			hitDirection = (RightDot > 0.0f) ? EHitDireciton::EHD_Right : EHitDireciton::EHD_Left;
		}
	}


}

void UGA_HitReact::OnMontageEnd(FGameplayEventData Payload)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_HitReact::EndHitReact()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}



