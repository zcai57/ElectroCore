// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_MontageEnd.generated.h"
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UAnimNotify_MontageEnd : public UAnimNotifyState
{
	GENERATED_BODY()
public:

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("MontageEnd");
	}
};
