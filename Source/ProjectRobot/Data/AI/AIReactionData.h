// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AbilitySystemComponent.h"
#include "AIReactionData.generated.h"


USTRUCT(BlueprintType)
struct FAIReactionRule
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery PlayerStateRequired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagQuery EnemyStateRequired;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ReactionEventTag; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag StateTagToApply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 1.0f;
};
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UAIReactionData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAIReactionRule> ReactionRules;
};
