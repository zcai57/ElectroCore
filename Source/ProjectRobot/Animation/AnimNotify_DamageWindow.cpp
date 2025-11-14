// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_DamageWindow.h"
#include "ProjectRobot/ActorComponents/AttackComponent.h"

void UAnimNotify_DamageWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		if (auto* Comp = Owner->GetComponentByClass<UAttackComponent>())
		{
			Comp->StartDamageTrace(WindowID, AttackTraceData);
		}
	}
}

void UAnimNotify_DamageWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr) {
		if (auto* Comp = Owner->FindComponentByClass<UAttackComponent>())
		{
			Comp->EndDamageTrace(AttackTraceData);
		}
	}

}