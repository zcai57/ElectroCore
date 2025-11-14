// Fill out your copyright notice in the Description page of Project Settings.
#include "GA_LightAtkCombo.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
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
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.AI.LightComboAtk"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}


void UGA_LightAtkCombo::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// if triggered from event
	if (TriggerEventData)
		MaxCombo = (int32)TriggerEventData->EventMagnitude;

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
	if (ComboPhase >= MaxCombo) return;
	bComboAdvancing = true;
	
	if (CurrentActorInfo && CurrentActorInfo->AvatarActor.IsValid())
	{
		AEnemy* EnemyActor = Cast<AEnemy>(CurrentActorInfo->AvatarActor.Get());
		UAnimMontage* MontageToPlay = ComboMontage1;
		if (ACharacter* Char = Cast<ACharacter>(CurrentActorInfo->AvatarActor))
		{
			UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance();
			check(Anim);
			if (ComboPhase == 0)
			{
				/*Char->PlayAnimMontage(ComboMontage1, 1.f);*/
				MontageToPlay = ComboMontage1;
				if (EnemyActor)
				{
					EnemyActor->ClampMotionWarpDist(MotionWarpDist1);
				}
			}
			else if (ComboPhase == 1)
			{
				/*Char->PlayAnimMontage(ComboMontage2, 1.f);*/
				MontageToPlay = ComboMontage2;
				if (EnemyActor)
				{
					EnemyActor->ClampMotionWarpDist(MotionWarpDist2);
				}
			}
			else if (ComboPhase == 2)
			{
				/*Char->PlayAnimMontage(ComboMontage3, 1.f);*/
				MontageToPlay = ComboMontage3;
				if (EnemyActor)
				{
					EnemyActor->ClampMotionWarpDist(MotionWarpDist3);
				}
			}
			else if (ComboPhase == 3)
			{
				/*Char->PlayAnimMontage(ComboMontage4, 1.f);*/
				MontageToPlay = ComboMontage4;
				if (EnemyActor)
				{
					EnemyActor->ClampMotionWarpDist(MotionWarpDist4);
				}
			}
			
			// if(AEnemyControllerBase* controller = Cast<AEnemyControllerBase>(EnemyActor->GetController()))
			// {
			// 	controller->FaceCombatTarget();
			// }
			
			ComboPhase += 1;
		}
		// Active MontageTask
		ActiveTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageToPlay, 1.0f, NAME_None, /*bStopWhenAbilityEnds*/ true, 1.0f);
		ActiveTask->ReadyForActivation();
	}
	bComboAdvancing = false;
}

void UGA_LightAtkCombo::OnDealDamage(FGameplayEventData Payload)
{
	//check(CurrentActorInfo);

	//const FHitResult* Hit = Payload.ContextHandle.GetHitResult();

	//AActor* Target = Hit->GetActor();
	//AActor* CurActor = CurrentActorInfo->AvatarActor.Get();
	//if (!Target) return;

	//// Send HitReact Event
	//if (Hit)
	//{
	//	Payload.EventTag = FGameplayTag::RequestGameplayTag("Event.HitReact");
	//	Payload.Target = Target;
	//	Payload.Instigator = GetAvatarActorFromActorInfo(); // the attacker
	//	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, Payload.EventTag, Payload);
	//}
	//

	//// Send HitStop Event
	//FGameplayEventData Data;
	//FGameplayTag hitStopTag = FGameplayTag::RequestGameplayTag("Event.HitStop");
	//UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, hitStopTag, Data);
	//
	//check(CurActor);
	//UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(CurrentActorInfo->AvatarActor.Get(), hitStopTag, Data);

	//// Execute LightAtkHit GameplayCue
	//UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get();
	//FGameplayCueParameters Params;
	//Params.Instigator = CurActor;                
	//Params.EffectCauser = Target;             
	//Params.Location = Hit->ImpactPoint;            
	//Params.Normal = Hit->ImpactNormal;             
	//Params.TargetAttachComponent = Target->FindComponentByClass<USkeletalMeshComponent>(); 

	//ASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag("GameplayCue.LightAttack"), Params);
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
