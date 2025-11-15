// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_LightHitReact.h"

UGA_LightHitReact::UGA_LightHitReact()
{
	FAbilityTriggerData Trigger;
	HitReactEventTag = FGameplayTag::RequestGameplayTag(FName("Event.LightHitReact"));
	Trigger.TriggerTag = HitReactEventTag;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}
