// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotify_CameraMovement.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UAnimNotify_CameraMovement : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector CameraBoomOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float OffsetInterpSpeed = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float OffsetResetSpeed = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CamerBoomLengthOffset = 0.f;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

};
