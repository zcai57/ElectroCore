// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_HeavyHitReact.h"

UGA_HeavyHitReact::UGA_HeavyHitReact()
{
	
	FAbilityTriggerData Trigger;
	HitReactEventTag = FGameplayTag::RequestGameplayTag(FName("Event.HeavyHitReact"));
	Trigger.TriggerTag = HitReactEventTag;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}
