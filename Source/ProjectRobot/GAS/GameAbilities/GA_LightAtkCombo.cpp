// Fill out your copyright notice in the Description page of Project Settings.
#include "GA_LightAtkCombo.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "ProjectRobot/Data/Attack/AttackDefinitionData.h"
#include "ProjectRobot/Enemy/Enemy.h"
#include "ProjectRobot/Enemy/EnemyControllerBase.h"


UGA_LightAtkCombo::UGA_LightAtkCombo()
{
	bIsCancelable = true;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Attacking"));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Stepping"));   // auto-added on activate, removed on end
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Immobile"));   // if you want to lock movement during the step

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dodge"));

	// Set up Trigger
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.MeleeAttack"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}


void UGA_LightAtkCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// if triggered from event
	if (TriggerEventData)
	{
		MaxCombo = (int32)TriggerEventData->EventMagnitude;
		if (TriggerEventData->OptionalObject)
		{
			if (const UAttackDefinitionData* Def = Cast<UAttackDefinitionData>(TriggerEventData->OptionalObject))
			{
				AttackData = Def;
			} else
			{
				UE_LOG(LogTemp, Error, TEXT("OptionalObject is not UAttackDefinitionData!"));
			}
		} 
	}
		
	

	// Reset state for safety
	bTakingInput = false;
	bAttacking = false;
	bComboAdvancing = false;
	
	// Check Avatar and Montage
	ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

	// Play animation and bind notify
	if (UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance())
	{

		// Bind gameplay events BEFORE montage starts
		{
			auto* BeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, FGameplayTag::RequestGameplayTag(FName("Event.Attack.ComboWindow.Begin")));
			BeginTask->EventReceived.AddDynamic(this, &UGA_LightAtkCombo::OnComboWindowBegin);
			BeginTask->ReadyForActivation();

			auto* EndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, FGameplayTag::RequestGameplayTag(FName("Event.Attack.ComboWindow.End")));
			EndTask->EventReceived.AddDynamic(this, &UGA_LightAtkCombo::OnComboWindowEnd);
			EndTask->ReadyForActivation();

			/*auto* EndComboTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
				this, FGameplayTag::RequestGameplayTag(FName("Event.MontageEnd")));
			EndComboTask->EventReceived.AddDynamic(this, &UGA_LightAtkCombo::OnMontageEnd);
			EndComboTask->ReadyForActivation();*/

		}

		/*Char->PlayAnimMontage(ComboMontage1, 1.f, FName("Light1"));*/
	}
	AdvanceCombo();
}

void UGA_LightAtkCombo::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ComboPhase = 0;
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

/// Advance combo when player have input pressed
/// @param Handle 
/// @param ActorInfo 
/// @param ActivationInfo 
void UGA_LightAtkCombo::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	check(!bAutoAdvance);
	RequestComboAdvance();
}


void UGA_LightAtkCombo::RequestComboAdvance()
{
	if (bTakingInput)
	{
		bAttacking = true;
		AdvanceCombo();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Input Pressed 0 "));
}

void UGA_LightAtkCombo::ApplyDamage()
{
	if (!CurrentActorInfo || !DamageEffectClass) return;

	AActor* Source = CurrentActorInfo->AvatarActor.Get();

}

void UGA_LightAtkCombo::OnComboWindowBegin(FGameplayEventData Payload)
{
	bTakingInput = true;
	if (bAutoAdvance)
	{
		RequestComboAdvance();
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Taking Input"));
}


void UGA_LightAtkCombo::OnComboWindowEnd(FGameplayEventData Payload)
{
	bTakingInput = false;
	// if comboWindow End, check Attacking state, if true reset, false EndAbility
	if (bAttacking)
	{
		bAttacking = false;
	}
	else {
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UGA_LightAtkCombo::OnMontageEnd(FGameplayEventData Payload)
{
	if (bComboAdvancing) return;
	Super::OnMontageEnd(Payload);
}

void UGA_LightAtkCombo::AdvanceCombo()
{
	if (!AttackData)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (ComboPhase >= AttackData->AttackDefinition.Num())
	{
		// No more steps, finish
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	bComboAdvancing = true;

	// Get the attack step safely
	const FAttackDefinition& Step = AttackData->AttackDefinition[ComboPhase];

	if (!Step.Montage)
	{
		UE_LOG(LogTemp, Error, TEXT("Combo step %d has no montage!"), ComboPhase);
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		bComboAdvancing = false;
		return;
	}

	AEnemy* EnemyActor = Cast<AEnemy>(CurrentActorInfo->AvatarActor.Get());

	if (EnemyActor)
	{
		EnemyActor->ClampMotionWarpDist(Step.MotionWarpDistance);
	}

	// Active MontageTask
	ActiveTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Step.Montage, 1.0f, NAME_None, /*bStopWhenAbilityEnds*/ true, 1.0f);
	ActiveTask->ReadyForActivation();
	
	ComboPhase += 1;
	bComboAdvancing = false;
}

void UGA_LightAtkCombo::OnDealDamage(FGameplayEventData Payload)
{
	Super::OnDealDamage(Payload);
}

void UGA_LightAtkCombo::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);

	// Safety: if montage still playing, stop it
	if (UAnimInstance* Anim = ActorInfo->GetAnimInstance())
	{
		if (Anim->IsAnyMontagePlaying())
			Anim->Montage_Stop(0.1f);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}
