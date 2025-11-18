// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GasWidgetBase.h"
#include "ResourceBarWidget.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UResourceBarWidget : public UGasWidgetBase
{
	GENERATED_BODY()

public:
	UResourceBarWidget();
	
	UFUNCTION(BlueprintCallable)
	void UpdateResource(float newValue);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* UnderBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* TopBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float InterpSpeed = 0.5f;

private:
	float TargetValue;
	float CurrentInterpValue;
};
