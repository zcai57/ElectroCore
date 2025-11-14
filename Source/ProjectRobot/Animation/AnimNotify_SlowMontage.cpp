// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_SlowMontage.h"
#include "GameFramework/Character.h"

void UAnimNotify_SlowMontage::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
		if (!Character) return;
		
		Character->CustomTimeDilation = SlowRate;
	}
}

void UAnimNotify_SlowMontage::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp && MeshComp->GetOwner())
	{
		ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
		if (!Character) return;

		Character->CustomTimeDilation = 1.0f;
	}
}
