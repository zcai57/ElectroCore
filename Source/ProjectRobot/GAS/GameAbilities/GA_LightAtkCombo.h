// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MontageAbility.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GA_LightAtkCombo.generated.h"

class UAnimMontage;
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UGA_LightAtkCombo : public UGA_MontageAbility
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	UAnimMontage* ComboMontage1 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	UAnimMontage* ComboMontage2 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	UAnimMontage* ComboMontage3 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	UAnimMontage* ComboMontage4 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	float MotionWarpDist1 = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	float MotionWarpDist2 = 200.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	float MotionWarpDist3 = 250.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	float MotionWarpDist4 = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	int32 MaxCombo = 4;

	UPROPERTY(EditDefaultsOnly, Category = "Combo")
	bool bAutoAdvance = false;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGameplayTag Data_DamageTag;
public:
	UGA_LightAtkCombo();

	void RequestComboAdvance();
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) override;
	void ApplyDamage();

	UFUNCTION()
	void OnComboWindowBegin(FGameplayEventData Payload);

	UFUNCTION()
	void OnComboWindowEnd(FGameplayEventData Payload);

	void OnMontageEnd(FGameplayEventData Payload) override;

	void OnDealDamage(FGameplayEventData Payload) override;

	void CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility) override;

	void AdvanceCombo();
private:
	int32 ComboPhase = 0;
	bool bTakingInput = false;
	bool bAttacking = false;
	bool bComboAdvancing = false;
	UAbilityTask_PlayMontageAndWait* ActiveTask = nullptr;
};
