// Fill out your copyright notice in the Description page of Project Settings.
#include "JetMoveComponent.h"


void UJetMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner && !UpdatedComponent && ShouldSkipUpdate(DeltaTime)) return;

	switch (JetMovementMode)
	{
	case EJetMovementMode::Jet_Slide:
		SimulateSlide(DeltaTime);
		break;

	case EJetMovementMode::Jet_Dash:
		SimulateDash(DeltaTime);
		break;

	case EJetMovementMode::Jet_Thrust:
		break;

	default:
		break;
	}
}

void UJetMoveComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
}

void UJetMoveComponent::SetJetMovementMode(EJetMovementMode mode)
{
	JetMovementMode = mode;
}

void UJetMoveComponent::SimulateSlide(float DeltaTime)
{
	FVector Input = ConsumeInputVector();
	FString DebugText = FString::Printf(TEXT("InputVector: %s"), *Input.ToString());
	GEngine->AddOnScreenDebugMessage(2, 10.f, FColor::Red, DebugText);

	// Update WindResistance based on pitch
	check(UpdatedComponent);
	FVector ForwardVector = UpdatedComponent->GetForwardVector();
	float WindResistFactor = FMath::Clamp(FVector::DotProduct(ForwardVector, FVector::DownVector), 0.f, 1.f);
	float WindResistCurr = Slide_WindResistance * WindResistFactor;
	
	// Compute accumulated X speed and interp to speed
	if (Input.X != 0.f) {
		Slide_AccumulateSpeedX += Input.X * (Slide_MaxSpeedX - Slide_MinSpeedX) * DeltaTime;
	}
	else {
		// No input gradually lose speed
		Slide_AccumulateSpeedX -= Slide_LoseSpeedX * DeltaTime;
	}
	Slide_AccumulateSpeedX = FMath::Clamp(Slide_AccumulateSpeedX, Slide_MinSpeedX, Slide_MaxSpeedX);

	// Velocity X
	Velocity.X = Slide_AccumulateSpeedX;
	FString DebugText1 = FString::Printf(TEXT("VelocityX: %s"), *Velocity.ToString());
	GEngine->AddOnScreenDebugMessage(3, 10.f, FColor::Red, DebugText1);
	// Z Drag
	Velocity.Z -= Gravity * DeltaTime;
	Velocity.Z *= FMath::Clamp(1.0f - (WindResistCurr * DeltaTime), 0.f, 1.f);

	// Convert Velocity to world space
	FRotator Rotation = UpdatedComponent->GetComponentRotation();
	FVector WorldVelocity = Rotation.RotateVector(Velocity);

	//UE_LOG(LogTemp, Warning, TEXT("Jet Velocity: %s"), *Velocity.ToString());
	FHitResult Hit;
	SafeMoveUpdatedComponent(WorldVelocity * DeltaTime, Rotation, true, Hit);
	
	if (Hit.IsValidBlockingHit())
	{
		HandleImpact(Hit);
	}
}

void UJetMoveComponent::SimulateThrust(float DeltaTime)
{
}

void UJetMoveComponent::SimulateDash(float DeltaTime)
{
}
