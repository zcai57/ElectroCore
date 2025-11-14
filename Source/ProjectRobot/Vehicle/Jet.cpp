// Fill out your copyright notice in the Description page of Project Settings.


#include "Jet.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"
#include "../MoveComponents/JetMoveComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AJet::AJet()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set up root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));


	// Set up Jet mesh
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("JetMesh"));
	Mesh->SetupAttachment(RootComponent);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Init Custom Movement Component
	JetMovementComponent = CreateDefaultSubobject<UJetMoveComponent>(TEXT("JetMoveComponent"));
	JetMovementComponent->SetUpdatedComponent(RootComponent);
}

// Called when the game starts or when spawned
void AJet::BeginPlay()
{
	Super::BeginPlay();
	// Set Initial Movement mode
	check(JetMovementComponent);
	if (JetMovementComponent)
	{
		JetMovementComponent->SetJetMovementMode(EJetMovementMode::Jet_Slide);
	}
}

void AJet::Pitch(const FInputActionValue& Value)
{
	// input is a Vector2D
	float PitchAxis = Value.Get<float>();
	check(PitchAxis != 0.f);

	FRotator CurrRotation = Controller->GetControlRotation();
	// Get target pitch
	CurrRotation.Pitch += PitchAxis * CameraLookSensitivity;
	CurrRotation.Pitch = FMath::Clamp(CurrRotation.Pitch, -60, 0);

	// Interpolation
	//CurrRotation.Pitch = FMath::FInterpTo(CurrRotation.Pitch, DesiredPitch, DeltaTime, PitchInterpSpeed);

	Controller->SetControlRotation(CurrRotation);
}

void AJet::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	GEngine->AddOnScreenDebugMessage(1, 10.f, FColor::Red, FString("Moving"));

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();

		// Flatten rotation, zero out pitch and roll
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.X * AccerlerateFactor);
		AddMovementInput(RightDirection, MovementVector.Y * StrafeFactor);
	}
}

// Called every frame
void AJet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AJet::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(PitchAction, ETriggerEvent::Triggered, this, &AJet::Pitch);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AJet::Move);
		EnhancedInputComponent->BindAction(SwitchAction, ETriggerEvent::Triggered, this, &AJet::SwitchBackControl);
	}

}

void AJet::SwitchBackControl()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (prevPawn)
	{
		// Hide and disable collision for current actor
		this->SetActorEnableCollision(false);
		this->SetActorHiddenInGame(true);
		// Set locationa and switch control back to prevPawn
		prevPawn->SetActorLocation(this->GetActorLocation());
		prevPawn->SetActorHiddenInGame(false);
		prevPawn->SetActorEnableCollision(true);
		prevPawn->DisableInput(PC);
		prevPawn->EnableInput(PC);
		PC->Possess(prevPawn);

		// Optionally: delay destroyf
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AJet::DestroySelf, 0.2f, false);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Couldn't find prev pawn"));
	}
}

void AJet::Init(APawn* prev)
{
	check(!prevPawn)
	prevPawn = prev;
}

void AJet::DestroySelf()
{
	if (this)
	{
		this->Destroy();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Trying to destroy JetActor that is null!"));
	}
}

