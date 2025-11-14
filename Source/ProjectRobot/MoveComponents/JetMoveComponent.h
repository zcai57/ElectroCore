// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "JetMoveComponent.generated.h"

UENUM(BlueprintType)
enum class EJetMovementMode : uint8
{
	Jet_None UMETA(DisplayName = "None"),
	Jet_Slide UMETA(DisplayName = "Slide"),
	Jet_Thrust UMETA(DisplayName = "Thrust"),
	Jet_Dash UMETA(DisplayName = "Dash"),
};
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API UJetMoveComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	EJetMovementMode JetMovementMode = EJetMovementMode::Jet_Slide;
public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
	
	UFUNCTION(BlueprintCallable)
	void SetJetMovementMode(EJetMovementMode mode);
private:
	void SimulateSlide(float DeltaTime);
	void SimulateThrust(float DeltaTime);
	void SimulateDash(float DeltaTime);
private:
	UPROPERTY(EditDefaultsOnly) float Slide_MinSpeedX = 200.f;
	UPROPERTY(EditDefaultsOnly) float Slide_MaxSpeedX = 2000.f;
	UPROPERTY(EditDefaultsOnly) float Slide_WindResistance = 0.8f;
	UPROPERTY(EditDefaultsOnly) float Slide_AccelerateX = 0.8f;

	UPROPERTY(EditDefaultsOnly) float Slide_AccumulateSpeedX = 200.f;

	UPROPERTY(EditDefaultsOnly) float Slide_LoseSpeedX = 200.f;

	UPROPERTY(EditDefaultsOnly) float Thrust_ForwardForce;
	UPROPERTY(EditDefaultsOnly) float Thrust_UpwardForce;
	UPROPERTY(EditDefaultsOnly) float Thrust_DownwardForce;
	UPROPERTY(EditDefaultsOnly) float Thrust_RightwardForce;
	UPROPERTY(EditDefaultsOnly) float Thrust_LeftwardForce;

	UPROPERTY(EditDefaultsOnly) float Gravity = 9.8f;

};
