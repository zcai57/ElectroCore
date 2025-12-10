// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_AttackEffect.h"
#include "GameplayEffect.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ProjectRobot/Enemy/Enemy.h"
#include "ProjectRobot/Data/Attack/AttackTraceData.h"

UGA_AttackEffect::UGA_AttackEffect()
{

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = FGameplayTag::RequestGameplayTag(FName("Event.AttackEffect"));
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UGA_AttackEffect::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ApplyDamage(TriggerEventData);
	ApplyBuff(TriggerEventData);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UGA_AttackEffect::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_AttackEffect::ApplyDamage(const FGameplayEventData* TriggerEventData)
{
	// This is pretty bad right now!
	const AEnemy* EnemyActor = Cast<AEnemy>(TriggerEventData->Target);
	if (EnemyActor)
	{
		EnemyActor->OnDamageTaken(*TriggerEventData);
	}
	
	if (TriggerEventData)
	{
		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TriggerEventData->Target);
		if (!TargetASC || !TriggerEventData)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
			return;
		}
	
		const UAttackTraceData* AttackData = Cast<UAttackTraceData>(TriggerEventData->OptionalObject);
		
		FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(AttackData->DamageEffect, 1.0f);
		Spec.Data->SetSetByCallerMagnitude(AttackData->EnergyTag, AttackData->DamageToEnergy);
		Spec.Data->SetSetByCallerMagnitude(AttackData->HeatTag, AttackData->DamageToHeat);
		
		// If target is blocking → cancel or reduce damage
		if (TargetASC->HasMatchingGameplayTag(BlockTag))
		{
			Spec.Data->SetSetByCallerMagnitude(AttackData->EnergyTag, 0);
			Spec.Data->SetSetByCallerMagnitude(AttackData->HeatTag, 0);
			// Trigger Block Reaction
			TriggerBlockReact(TriggerEventData);
		}

		TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}

void UGA_AttackEffect::ApplyBuff(const FGameplayEventData* TriggerEventData)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	check(ASC);
	
	if (TriggerEventData)
	{
		const UAttackTraceData* AttackData = Cast<UAttackTraceData>(TriggerEventData->OptionalObject);
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(AttackData->BuffEffect, 1.0f, ASC->MakeEffectContext());
		Spec.Data->SetSetByCallerMagnitude(AttackData->EnergyTag, AttackData->BuffToEnergy);
		Spec.Data->SetSetByCallerMagnitude(AttackData->HeatTag, AttackData->BuffToHeat); // No buff to heat

		ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data);
	}
}

void UGA_AttackEffect::TriggerBlockReact(const FGameplayEventData* TriggerEventData)
{
	FGameplayEventData ReactPayload= *TriggerEventData;
	ReactPayload.EventTag = BlockReactTag;

	const AActor* TargetActor = TriggerEventData->Target;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(const_cast<AActor*>(TargetActor), ReactPayload.EventTag, ReactPayload);
}
