#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "GA_MontageAbility.generated.h"

UCLASS()
class PROJECTROBOT_API UGA_MontageAbility: public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MontageAbility();
protected:
	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	virtual void OnMontageEnd(FGameplayEventData Payload);

	UFUNCTION()
	virtual void OnDealDamage(FGameplayEventData Payload);

	UFUNCTION(BlueprintNativeEvent)
	void K2_OnPostDealDamage(const FGameplayEventData& Payload, const FHitResult& Hit, AActor* Target, AActor* InstigatorActor);
	virtual void K2_OnPostDealDamage_Implementation(const FGameplayEventData& Payload, const FHitResult& Hit, AActor* Target, AActor* InstigatorActor);
};

