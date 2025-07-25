// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverBike.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AHoverBike::AHoverBike()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Set up root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));


	// Set up hoverbike mesh
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BikeMesh"));
	Mesh->SetupAttachment(RootComponent);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Set up interact sphere
	InteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionVolume"));
	InteractionSphere->SetupAttachment(RootComponent);
	InteractionSphere->SetSphereRadius(400.f);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Set up Interactable Component
	InteractableComponent = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComponent"));

	// Set up Interact Widget
	InteractWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Interact UI"));
	InteractWidget->SetupAttachment(RootComponent);
	InteractWidget->SetWidgetSpace(EWidgetSpace::World);
	InteractWidget->SetDrawSize(FVector2D(640, 640));
	InteractWidget->SetRelativeLocation(FVector(0, 0, 100));
}

void AHoverBike::Interact_Implementation(AActor* interactActor)
{

}

// Called when the game starts or when spawned
void AHoverBike::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHoverBike::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

