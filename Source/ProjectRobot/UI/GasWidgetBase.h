// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilitySystemComponent.h"
#include "GasWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UGasWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "GAS")
    void InitFromASC(UAbilitySystemComponent* InASC);

    // For pawn swaps / re-init
    UFUNCTION(BlueprintCallable, Category = "GAS")
    void UnbindAllAttributes();
	
protected:
    // BP event: fires on any attribute change (implement per-widget)
    UFUNCTION(BlueprintImplementableEvent, Category = "GAS")
    void K2_OnAttributeChanged(FGameplayAttribute Attribute, float NewValue, float OldValue);

    // Call once per attribute you care about
    UFUNCTION(BlueprintCallable, Category = "GAS")
    void RegisterAttribute(FGameplayAttribute Attribute, bool bFireInitial = true);

    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintReadOnly, Category = "GAS")
    TObjectPtr<UAbilitySystemComponent> ASC;

private:
    // Attribute -> delegate handle
    TMap<FGameplayAttribute, FDelegateHandle> Handles;

    void HandleAttrChanged(const FOnAttributeChangeData& Data);
};
