// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldSpaceWidgetBase.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UWorldSpaceWidgetBase::UWorldSpaceWidgetBase()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWorldSpaceWidgetBase::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWorldSpaceWidgetBase::TickComponent(float DeltaTime, ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC || !PC->PlayerCameraManager)
		return;

	FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
	FVector WidgetLocation = GetComponentLocation();

	FVector Direction = CameraLocation - WidgetLocation;
	Direction.Z = 0;  // Make it 2D (optional)

	FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
	SetWorldRotation(LookAtRotation);
}

