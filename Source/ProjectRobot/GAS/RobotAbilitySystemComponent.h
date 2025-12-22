// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "RobotAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API URobotAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
	void RemoveAbilityBySlot(FGameplayTag SlotTag);
	void GrantAbilityToSlot(FGameplayTag SlotTag, TSubclassOf<UGameplayAbility> AbilityClass);
	
private:
	TMap<FGameplayTag, FGameplayAbilitySpecHandle> GrantedAbilityHandles;
};
