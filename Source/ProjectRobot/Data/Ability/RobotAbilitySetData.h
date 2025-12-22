// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/DataAsset.h"
#include "ProjectRobot/Data/Attack/AttackDefinitionData.h"
#include "RobotAbilitySetData.generated.h"


/**
 * @class URobotAbilitySetData
 * @brief A data asset used to define a set of robot abilities.
 *
 * Represents a customizable and reusable set of abilities for a robot entity in the PROJECTROBOT framework.
 * This class extends the UPrimaryDataAsset, allowing it to be used as a primary data asset for defining various ability sets.
 * Typically used within the gameplay framework to specify and manage functionalities and capabilities of in-game robots.
 */
UCLASS()
class PROJECTROBOT_API URobotAbilitySetData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag, TSubclassOf<UGameplayAbility>> Slots;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAttackDefinitionData* LightAtkData;
};
