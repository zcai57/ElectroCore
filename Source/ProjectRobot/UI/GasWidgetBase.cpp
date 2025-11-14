// Fill out your copyright notice in the Description page of Project Settings.


#include "GasWidgetBase.h"

void UGasWidgetBase::InitFromASC(UAbilitySystemComponent* InASC)
{
    if (ASC == InASC) return;
    UnbindAllAttributes();
    ASC = InASC;
}

void UGasWidgetBase::RegisterAttribute(FGameplayAttribute Attribute, bool bFireInitial)
{
    if (!ASC || !Attribute.IsValid() || Handles.Contains(Attribute)) return;

    auto& Delegate = ASC->GetGameplayAttributeValueChangeDelegate(Attribute);
    FDelegateHandle H = Delegate.AddUObject(this, &UGasWidgetBase::HandleAttrChanged);
    Handles.Add(Attribute, H);

    if (bFireInitial)
    {
        const float Val = ASC->GetNumericAttribute(Attribute);
        K2_OnAttributeChanged(Attribute, Val, Val);
    }
}

void UGasWidgetBase::NativeDestruct()
{
    UnbindAllAttributes();
    Super::NativeDestruct();
}

void UGasWidgetBase::HandleAttrChanged(const FOnAttributeChangeData& Data)
{
    K2_OnAttributeChanged(Data.Attribute, Data.NewValue, Data.OldValue);
}

void UGasWidgetBase::UnbindAllAttributes()
{
    if (!ASC) { Handles.Empty(); return; }
    for (auto& Pair : Handles)
        ASC->GetGameplayAttributeValueChangeDelegate(Pair.Key).Remove(Pair.Value);
    Handles.Empty();
}
