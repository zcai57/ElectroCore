#include "GA_MontageAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"

UGA_MontageAbility::UGA_MontageAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UGA_MontageAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	// Commit ability, if fail, end
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Check Avatar and Montage
	ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (!Char)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	auto* EndMontageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag(FName("Event.MontageEnd")));
	EndMontageTask->EventReceived.AddDynamic(this, &UGA_MontageAbility::OnMontageEnd);
	EndMontageTask->ReadyForActivation();

	auto* DealDamageTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, FGameplayTag::RequestGameplayTag(FName("Event.Attack.DealDmg")));
	DealDamageTask->EventReceived.AddDynamic(this, &UGA_MontageAbility::OnDealDamage);
	DealDamageTask->ReadyForActivation();
}

void UGA_MontageAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// Unbind Notify
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (ACharacter* Char = Cast<ACharacter>(ActorInfo->AvatarActor))
		{
			if (UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance())
			{
				Anim->OnPlayMontageNotifyBegin.RemoveAll(this);
				Anim->StopAllMontages(0.5f);
			}
		}
	}
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_MontageAbility::OnMontageEnd(FGameplayEventData Payload)
{
	//if (Payload.OptionalObject2 != this)
	//{
	//	return; // ignore: was sent by some other ability's montage
	//}
	//if (Payload.TargetTags.HasTag(FGameplayTag::RequestGameplayTag("State.Attacking")))
	//{
	//}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MontageAbility::OnDealDamage(FGameplayEventData Payload)
{
	check(CurrentActorInfo);

	const FHitResult* Hit = Payload.ContextHandle.GetHitResult();

	AActor* Target = Hit->GetActor();
	AActor* CurActor = CurrentActorInfo->AvatarActor.Get();
	if (!Target) return;

	// Send HitReact Event
	if (Hit)
	{
		Payload.EventTag = FGameplayTag::RequestGameplayTag("Event.HitReact");
		Payload.Target = Target;
		Payload.Instigator = GetAvatarActorFromActorInfo(); // the attacker
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, Payload.EventTag, Payload);
	}
	
	// Send HitStop Event
	FGameplayEventData Data;
	FGameplayTag hitStopTag = FGameplayTag::RequestGameplayTag("Event.HitStop");
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Target, hitStopTag, Data);

	check(CurActor);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(CurrentActorInfo->AvatarActor.Get(), hitStopTag, Data);

	// --- Post hook for child abilities (e.g., Dodge-Attack adds a special GE, camera pulse, tag, etc.) ---
	K2_OnPostDealDamage(Payload, *Hit, Target, CurActor);
}

void UGA_MontageAbility::K2_OnPostDealDamage_Implementation(const FGameplayEventData& Payload, const FHitResult& Hit, AActor* Target, AActor* InstigatorActor)
{
}
