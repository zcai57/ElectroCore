// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ProjectRobot/Data/DamageSpec.h"
#include "IDamageable.generated.h"

// This class does not need to be modified.

UINTERFACE(MinimalAPI)
class UIDamageable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECTROBOT_API IIDamageable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool CanReceiveDamage(const struct FDamageSpec& Spec) const = 0;
	virtual float ApplyDamage(const FDamageSpec& Spec) = 0; // return damage applied
};
