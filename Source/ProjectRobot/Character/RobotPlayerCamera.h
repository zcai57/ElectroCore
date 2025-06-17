// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "RobotPlayerCamera.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API ARobotPlayerCamera: public APlayerCameraManager
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly) float CrouchBlendDuration = .5f;
	float CrouchBlendTime;
public:
	ARobotPlayerCamera();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
};
