// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_ComboWindow.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ProjectRobot/ActorComponents/AttackComponent.h"

void UAnimNotify_ComboWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		// GAS
		FGameplayEventData Data;
		Data.EventTag = FGameplayTag::RequestGameplayTag("Event.Attack.ComboWindow.Begin");
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Data.EventTag, Data);

		/*if (auto* Comp = Owner->GetComponentByClass<UAttackComponent>())
		{
			Comp->OpenComboWindow(GraceBefore, TotalDuration + GraceAfter);
		}*/
	}
}

void UAnimNotify_ComboWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr) {
		// GAS
		FGameplayEventData Data;
		Data.EventTag = FGameplayTag::RequestGameplayTag("Event.Attack.ComboWindow.End");
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, Data.EventTag, Data);

		/*if (auto* Comp = Owner->FindComponentByClass<UAttackComponent>())
		{
			
		}*/
	}
		
}
