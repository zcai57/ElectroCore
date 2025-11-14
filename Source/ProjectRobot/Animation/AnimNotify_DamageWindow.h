// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectRobot/Data/AttackTraceData.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_DamageWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UAnimNotify_DamageWindow : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	FName WindowID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trace")
	TObjectPtr<UAttackTraceData> AttackTraceData;
};
