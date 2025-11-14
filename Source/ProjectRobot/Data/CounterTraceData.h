// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TraceWindow.h"
#include "CounterTraceData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UCounterTraceData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FTraceWindow TraceData;
};
