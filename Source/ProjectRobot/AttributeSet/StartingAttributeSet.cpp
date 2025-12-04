// Fill out your copyright notice in the Description page of Project Settings.


#include "StartingAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "ProjectRobot/Enemy/Enemy.h"

UStartingAttributeSet::UStartingAttributeSet()
{
	InitHeat(0.f);
	InitEnergy(0.f);
	InitStatus(0.f);
}

void UStartingAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
	{
		float NewEnergy = GetEnergy();
		float OldHealth = PrevEnergy;

		if (NewEnergy <= 0.f && OldHealth > 0.f)
		{
			AEnemy* Pawn = Cast<AEnemy>(GetOwningActor());
			if (Pawn) Pawn->OnDeathStart();
		}

		PrevEnergy = NewEnergy;
	}
}
