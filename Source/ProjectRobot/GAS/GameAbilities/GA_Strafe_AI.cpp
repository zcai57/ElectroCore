// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Strafe_AI.h"

UGA_Strafe_AI::UGA_Strafe_AI()
{
	bIsCancelable = true;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.AI.Strafe"));

	SourceBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.HitReact"));
	SourceBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Immobile"));
}

void UGA_Strafe_AI::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	
}

void UGA_Strafe_AI::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	
}
