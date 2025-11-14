// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "../Character/RobotPlayerCharacter.h"

// Sets default values for this component's properties
UInteractableComponent::UInteractableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// Get owner (the actor this component is attached to)
	AActor* Owner = GetOwner();
	if (!Owner) return;

	// Find interaction sphere
	TriggerZone = Owner->FindComponentByClass<USphereComponent>();
	if (!TriggerZone)
	{
		UE_LOG(LogTemp, Warning, TEXT("No USphereComponent found on %s"), *Owner->GetName());
		return;
	}

	// Bind tiggerZone's OnOverlap to component's OnOverlap
	TriggerZone->OnComponentBeginOverlap.AddDynamic(this, &UInteractableComponent::OnOverlapBegin);
	TriggerZone->OnComponentEndOverlap.AddDynamic(this, &UInteractableComponent::OnOverlapEnd);

	// Init widget component
	InteractWidget = Owner->FindComponentByClass<UWidgetComponent>();
	if (IsValid(InteractWidgetClass))
	{
		InteractWidget->SetWidgetClass(InteractWidgetClass);
	}
	InteractWidget->SetVisibility(false);

}

void UInteractableComponent::ShowInteractionPrompt()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interact Prompt"));
	InteractWidget->SetVisibility(true);
}

void UInteractableComponent::HideInteractionPrompt()
{
	InteractWidget->SetVisibility(false);
}

void UInteractableComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->IsA<ARobotPlayerCharacter>())
	{
		ShowInteractionPrompt();
		InteractedActor = OtherActor;
	}
}

void UInteractableComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor->IsA<ARobotPlayerCharacter>())
	{
		HideInteractionPrompt();
		InteractedActor = nullptr;
	}
}
// Called every frame
void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (InteractWidget && InteractWidget->IsVisible() && InteractedActor)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->PlayerCameraManager)
		{
			FVector CameraLocation = PC->PlayerCameraManager->GetCameraLocation();
			FVector WidgetLocation = InteractWidget->GetComponentLocation();
			FVector Direction = CameraLocation - WidgetLocation;

			FRotator LookAtRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
			InteractWidget->SetWorldRotation(LookAtRotation);
		}
	}
}

