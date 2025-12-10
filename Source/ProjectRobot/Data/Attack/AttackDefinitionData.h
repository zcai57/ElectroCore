// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttackDefinitionData.generated.h"


USTRUCT(BlueprintType)
struct FAttackDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float PlayRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MotionWarpDistance = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bTracking = true;
};
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UAttackDefinitionData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FAttackDefinition> AttackDefinition;
};
