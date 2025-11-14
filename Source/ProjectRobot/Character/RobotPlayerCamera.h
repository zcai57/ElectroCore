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

	// Call these from notifies
	//UFUNCTION(BlueprintCallable, Category = "CamRaise")
	//void ActivateCamRaise(FVector CameraOffset = FVector::ZeroVector, float InBlendInSpeed = -1.f, float InBlendOutSpeed = -1.f);

	//UFUNCTION(BlueprintCallable, Category = "CamRaise")
	//void DeactivateCamRaise();

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CamRaise")
	//float CameraOffsetBlendInTime = 6.f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CamRaise")
	//float CameraOffsetBlendOutTime = 6.f;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CamRaise")
	//FVector CameraOffset = FVector::ZeroVector;

private:
	//bool  bRaiseActive = false;
	//float Alpha = 0.f;
	//FVector CurrentOffset;
};
