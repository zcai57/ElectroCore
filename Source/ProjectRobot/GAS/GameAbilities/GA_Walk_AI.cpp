// Fill out your copyright notice in the Description page of Project Settings.


#include "GA_Walk_AI.h"
#include "GameFramework/Character.h"
// #include "AIController.h"
#include "AbilitySystemBlueprintLibrary.h"

UGA_Walk_AI::UGA_Walk_AI()
{
	bIsCancelable = true;

	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("State.AI.Walk"));

	SourceBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.HitReact"));
	SourceBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Immobile"));
}

void UGA_Walk_AI::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AAIController* AICon = Cast<AAIController>(ActorInfo->PlayerController);
	AActor* Owner = ActorInfo->AvatarActor.Get();

	if (!AICon || !Owner) return;
	// Get Potential End Location
	FVector MoveTarget;
	if (TriggerEventData && TriggerEventData->TargetData.Num() > 0)
	{
		MoveTarget = UAbilitySystemBlueprintLibrary::GetTargetDataEndPoint(TriggerEventData->TargetData, 0);
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("ActivateAbility returned NULL"));
	}
	// Get Potential TargetActor
	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());

	if (!TargetActor)
	{
		// Move to Location
		AICon->MoveToLocation(MoveTarget);
	} else
	{
		AICon->MoveToActor(TargetActor);
	}

	AICon->ReceiveMoveCompleted.AddDynamic(this, &UGA_Walk_AI::OnMoveCompleted);
}

void UGA_Walk_AI::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_Walk_AI::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

