// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_ComboWindow.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UAnimNotify_ComboWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float GraceBefore = 0.f; // open early
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combo")
	float GraceAfter = 0.f; // stay open after end

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("ComboWindow");
	}
};
