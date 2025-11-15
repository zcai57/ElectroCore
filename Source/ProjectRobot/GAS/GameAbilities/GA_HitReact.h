// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_HitReact.generated.h"

UENUM()
enum class EHitDireciton : uint8
{
	EHD_None UMETA(Display = "NoDireciton"),
	EHD_Forward UMETA(DisplayName = "Forward"),
	EHD_Back UMETA(DisplayName = "Back"),
	EHD_Left UMETA(DisplayName = "Left"),
	EHD_Right UMETA(DisplayName = "Right"),
};

class UAbilityTask_PlayMontageAndWait;
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UGA_HitReact : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_HitReact();

	// The gameplay event that should trigger this ability (e.g., "Event.HitReact")
	UPROPERTY(EditDefaultsOnly, Category = "HitReact|Config")
	FGameplayTag HitReactEventTag;

	UPROPERTY(EditDefaultsOnly, Category = "HitReact|Anim")
	UAnimMontage* HitReactForward = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "HitReact|Anim")
	UAnimMontage* HitReactBack = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "HitReact|Anim")
	UAnimMontage* HitReactLeft = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "HitReact|Anim")
	UAnimMontage* HitReactRight = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "HitReact|Anim")
	float ZKick = 20.f;

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
protected:

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	void GetHitDirection(const FVector& Hit);

	UFUNCTION()
	void OnMontageEnd(FGameplayEventData Payload);

	UFUNCTION()
	void EndHitReact();

private:
	EHitDireciton hitDirection = EHitDireciton::EHD_None;
	UPROPERTY()
	UAbilityTask_PlayMontageAndWait* ActiveTask = nullptr;
};
