// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DamageSpec.h"
#include "TraceWindow.h"
#include "AttackTraceData.generated.h"

UENUM(BlueprintType)
enum class EAttackDeliveryType : uint8
{
	Melee,
	Projectile,
	AOE
};
/**
 * 
 */
UCLASS(BlueprintType)
class PROJECTROBOT_API UAttackTraceData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTraceWindow TraceData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EAttackDeliveryType DeliveryType = EAttackDeliveryType::Melee;
};