// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_MontageEnd.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"

void UAnimNotify_MontageEnd::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{

}

void UAnimNotify_MontageEnd::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr) {
		// GAS
		FGameplayEventData Data;
		// Pass in all current tag
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Owner))
		{
			if (const UGameplayAbility* AnimGA = ASC->GetAnimatingAbility())
			{
				Data.OptionalObject2 = const_cast<UGameplayAbility*>(AnimGA);
			}
		}
		Data.EventTag = FGameplayTag::RequestGameplayTag("Event.MontageEnd");
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Data.EventTag, Data);
	}

}