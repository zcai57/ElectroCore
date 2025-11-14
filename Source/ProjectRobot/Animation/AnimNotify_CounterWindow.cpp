// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_CounterWindow.h"

#include "ProjectRobot/ProjectRobotCharacter.h"
#include "ProjectRobot/ActorComponents/AttackComponent.h"
#include "ProjectRobot/Character/RobotPlayerCharacter.h"
#include "ProjectRobot/Data/CounterTraceData.h"

void UAnimNotify_CounterWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		if (auto* Comp = Owner->GetComponentByClass<UAttackComponent>())
		{
			Comp->StartCounterTrace(CounterTraceData->TraceData);
			if (ARobotPlayerCharacter* ACharacter = Cast<ARobotPlayerCharacter>(Owner))
			{
				ACharacter->SetInvulnerability(true);
			}
		}
	}
}

void UAnimNotify_CounterWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr) {
		if (auto* Comp = Owner->FindComponentByClass<UAttackComponent>())
		{
			Comp->EndCounterTrace();
			if (ARobotPlayerCharacter* ACharacter = Cast<ARobotPlayerCharacter>(Owner))
			{
				ACharacter->SetInvulnerability(false);
			}
		}
	}

}

