// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotAbilitySystemComponent.h"
#include "GameAbilities/GA_LightAtkCombo.h"

void URobotAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		const UGameplayAbility* GA = AbilityClass.GetDefaultObject();
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (GA && GA->AbilityTags.HasTag(FGameplayTag::RequestGameplayTag("Input.Action.LightCombo")))
		{
			AbilitySpec.InputID = 0;
		}
		GiveAbility(AbilitySpec);
	}
}


void URobotAbilitySystemComponent::RemoveAbilityBySlot(FGameplayTag SlotTag)
{
	if (FGameplayAbilitySpecHandle* HandlePtr = GrantedAbilityHandles.Find(SlotTag))
	{
		ClearAbility(*HandlePtr);
		GrantedAbilityHandles.Remove(SlotTag);
	}
}

void URobotAbilitySystemComponent::GrantAbilityToSlot(FGameplayTag SlotTag, TSubclassOf<UGameplayAbility> AbilityClass)
{
	if (!AbilityClass)
		return;
	
	FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE);
	if (SlotTag == FGameplayTag::RequestGameplayTag("AbilitySet.LightAtk"))
	{
		AbilitySpec.InputID = 0;
	}
	FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);

	GrantedAbilityHandles.Add(SlotTag, Handle);
}





