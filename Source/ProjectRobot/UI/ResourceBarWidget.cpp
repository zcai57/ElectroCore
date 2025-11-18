// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceBarWidget.h"
#include "Components/ProgressBar.h"

UResourceBarWidget::UResourceBarWidget()
{
	CurrentInterpValue = 1.f;
}

void UResourceBarWidget::UpdateResource(float newValue)
{
	TargetValue = newValue;
	TopBar->SetPercent(newValue);
}

void UResourceBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (UnderBar->Percent > TargetValue)
	{
		float newValue = FMath::FInterpTo(CurrentInterpValue, TargetValue, InDeltaTime, InterpSpeed);;
		UnderBar->SetPercent(newValue);
		CurrentInterpValue = newValue;
	}
	
}
