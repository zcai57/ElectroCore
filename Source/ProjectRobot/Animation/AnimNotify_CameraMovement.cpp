// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_CameraMovement.h"
#include "../Character/RobotPlayerCharacter.h"
#include "GameFramework/Character.h"

void UAnimNotify_CameraMovement::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (ACharacter* C = Cast<ACharacter>(MeshComp->GetOwner())) {
		if (ARobotPlayerCharacter* PC = Cast<ARobotPlayerCharacter>(C)) {
			PC->StartCameraMove(CameraBoomOffset, CamerBoomLengthOffset, OffsetInterpSpeed, OffsetResetSpeed);
		}
	}
}

void UAnimNotify_CameraMovement::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (ACharacter* C = Cast<ACharacter>(MeshComp->GetOwner())) {
		if (ARobotPlayerCharacter* PC = Cast<ARobotPlayerCharacter>(C)) {
			PC->StopCameraMove();
		}
	}
}
