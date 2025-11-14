// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StartingAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName)\
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UStartingAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UStartingAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Heat;
	ATTRIBUTE_ACCESSORS(UStartingAttributeSet, Heat);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Energy;
	ATTRIBUTE_ACCESSORS(UStartingAttributeSet, Energy);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Status;
	ATTRIBUTE_ACCESSORS(UStartingAttributeSet, Status);
};
