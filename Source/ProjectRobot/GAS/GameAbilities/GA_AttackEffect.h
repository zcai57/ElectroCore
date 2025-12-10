// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_AttackEffect.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UGA_AttackEffect : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UGA_AttackEffect();
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	void ApplyDamage(const FGameplayEventData* TriggerEventData);

	void ApplyBuff(const FGameplayEventData* TriggerEventData);

	void TriggerBlockReact(const FGameplayEventData* TriggerEventData);
	
private:
	UPROPERTY()
	FGameplayTag BlockTag = FGameplayTag::RequestGameplayTag("State.Block");
	UPROPERTY()
	FGameplayTag BlockReactTag = FGameplayTag::RequestGameplayTag("Event.BlockReact");
};
