// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MontageAbility.h"
#include "GA_Dodge.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UGA_Dodge : public UGA_MontageAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeF = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeB = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeFL = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeFR = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeL = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeR = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeBL = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	UAnimMontage* DodgeBR = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float XYstrength = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float Zstrength = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	bool bEnableDebug = true;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	TSubclassOf<UGameplayEffect> DodgeWindowEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	float DodgeDuration = 0.35f;

	FActiveGameplayEffectHandle DodgeGEHandle;

	UGA_Dodge();
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void GetDodgeDirection();

	void ChooseMontageAndPlay();

	void LaunchCharacter();

	void ExecuteGameplayCue();

	UFUNCTION()
	void EndDodge();

	UFUNCTION()
	void OnDodgeGE_Removed(const FGameplayEffectRemovalInfo& RemovalInfo);
private:
	FName DodgeDir = FName("F");
	FVector DodgeDirWS = FVector::ZeroVector;
};
