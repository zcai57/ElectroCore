// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RobotPlayerMovement.generated.h"

class ARobotPlayerCharacter;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None		UMETA(Hidden),
	CMOVE_Slide		UMETA(DisplayName = "Slide"),
	CMOVE_Boost		UMETA(DisplayName = "Boost"),
	CMOVE_MAX		UMETA(Hidden),
};

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API URobotPlayerMovement : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
	class FSavedMove_Robot : public FSavedMove_Character
	{
		typedef FSavedMove_Character Super;
		
		uint8 Saved_bWantsToSprint : 1;
		uint8 Saved_bPrevWantsToCrouch : 1;
		uint8 Saved_bWatnsToBoost : 1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData);
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_Robot : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Robot(const UCharacterMovementComponent& clientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};
	// Parameters
	UPROPERTY(EditDefaultsOnly) float Sprint_MaxWalkSpeed;
	UPROPERTY(EditDefaultsOnly) float Walk_MaxWalkSpeed;

	UPROPERTY(EditDefaultsOnly) float Slide_EnterSpeed = 500;
	UPROPERTY(EditDefaultsOnly) float Slide_ExitSpeed = 300;
	UPROPERTY(EditDefaultsOnly) float Slide_EnterImpulse = 500;
	UPROPERTY(EditDefaultsOnly) float Slide_GravityForce = 5000;
	UPROPERTY(EditDefaultsOnly) float Slide_Friction = 1.3;

	UPROPERTY(EditDefaultsOnly) float Boost_UpImpulse = 1000;
	UPROPERTY(EditDefaultsOnly) float Boost_ForwardImpulse = 300;
	UPROPERTY(EditDefaultsOnly) float Boost_MaxForwardVelocity = 800;
	UPROPERTY(EditDefaultsOnly) float Boost_GravityForce = 1000;
	
	
	// Transient
	UPROPERTY(Transient) ARobotPlayerCharacter* RobotCharacterOwner;

	bool Safe_bWantsToSprint;
	bool Safe_bPrevWantsToCrouch;
	bool Safe_bWantsToBoost;
protected:
	virtual void InitializeComponent() override;

public:
	bool bWantsToBoost;
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void OnMovementUpdated(float deltaSecond, const FVector& OldLocation, const FVector& OldVelocity) override;

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void PhysCustom(float deltaTime, int32 Iterations) override;

private:
	// Slide
	void EnterSlide();
	void ExitSlide();
	void PhysSlide(float deltaTime, int32 Iterations);
	bool GetSlideSurface(FHitResult& Hit) const;

	// Boost
	void EnterBoost();
	void ExitBoost();
	void PhysBoost(float deltaTIme, int32 Iterations);
	bool GetBoostCeiling(FHitResult& Hit) const;
public: 
	URobotPlayerMovement();
	UFUNCTION(BlueprintCallable) void SprintPressed();
	UFUNCTION(BlueprintCallable) void SprintReleased();
	UFUNCTION(BlueprintCallable) void BoostPressed();
	UFUNCTION(BlueprintCallable) void BoostReleased();
	UFUNCTION(BlueprintCallable) void CrouchPressed();
	
	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
};
