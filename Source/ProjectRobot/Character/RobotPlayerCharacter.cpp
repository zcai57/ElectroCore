// Fill out your copyright notice in the Description page of Project Settings.


#include "../Character/RobotPlayerCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "RobotPlayerMovement.h"

// Sockets
// FrontPelvisSocket
// BackPelvisSocket
// Spline3Socket
// RightHandSocket
// HeadSocket
// LeftThighSocket
// RightThighSocket

// Sets default values
ARobotPlayerCharacter::ARobotPlayerCharacter(const FObjectInitializer& ObjectInitializer) 
: Super(ObjectInitializer.SetDefaultSubobjectClass<URobotPlayerMovement>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Set up Apparel
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	Sword = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RobotSwordMesh"));
	Sword->SetupAttachment(GetMesh(), FName("RightHandSocket"));
	
	Cloth = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RobotClothMesh"));
	Cloth->SetupAttachment(GetMesh(), FName("Spline3Socket"));

	Hat = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HatMesh"));
	Hat->SetupAttachment(GetMesh(), FName("HeadSocket"));

	ShieldFront = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FrontShieldMesh"));
	ShieldBack = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BackShieldMesh"));
	ShieldFront->SetupAttachment(GetMesh(), FName("FrontPelvisSocket"));
	ShieldBack->SetupAttachment(GetMesh(), FName("BackPelvisSocket"));

	ShieldLeft = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LeftShieldMesh"));
	ShieldRight = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RightShieldMesh"));
	ShieldLeft->SetupAttachment(GetMesh(), FName("LeftThighSocket"));
	ShieldRight->SetupAttachment(GetMesh(), FName("RightThighSocket"));
	
}

// Called when the game starts or when spawned
void ARobotPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	RobotPlayerMovementComponent = Cast<URobotPlayerMovement>(GetCharacterMovement());
	SetDefaultApparel();
}

void ARobotPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bImmobile) return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		if (GetActorRotation() != Rotation)
		{
			SetActorRotation(FMath::RInterpTo(GetActorRotation(), Rotation, FApp::GetDeltaTime(), 0.3f));
		}
	}
}

void ARobotPlayerCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	/*if (LookAxisVector.X > 0)
	{
		CharacterDirection = ECharacterDirection::ECD_Forward;
	}
	else if (LookAxisVector.X < 0)
	{
		CharacterDirection = ECharacterDirection::ECD_Backward;
	}
	else if (LookAxisVector.Y < 0)
	{
		CharacterDirection = ECharacterDirection::ECD_Leftward;
	}
	else if (LookAxisVector.Y > 0)
	{
		CharacterDirection = ECharacterDirection::ECD_Rightward;
	}*/

	if (CameraBoom)
	{
		// Rotate CameraBoom using the Look input (X for Yaw, Y for Pitch)
		FRotator NewRotation = CameraBoom->GetRelativeRotation();
		NewRotation.Yaw += LookAxisVector.X * CameraLookSensitivity;
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - LookAxisVector.Y * CameraLookSensitivity, -60.f, 60.f); // Limit pitch range

		CameraBoom->SetRelativeRotation(NewRotation);
		const FRotator Rotation = Controller->GetControlRotation();
		Controller->SetControlRotation(NewRotation);
	}
}

void ARobotPlayerCharacter::Equip(const FInputActionValue& Value)
{
}

void ARobotPlayerCharacter::LightAttack(const FInputActionValue& Value)
{
	const bool canAttack = (ActionState == EActionState::EAS_Unoccupied);
	if (canAttack)
	{
		PlayLightAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
	
}

void ARobotPlayerCharacter::HeavyAttack(const FInputActionValue& Value)
{
}

void ARobotPlayerCharacter::Slide(const FInputActionValue& Value)
{

}

void ARobotPlayerCharacter::SprintPressed()
{
	if (RobotPlayerMovementComponent) {
		RobotPlayerMovementComponent->SprintPressed();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("RobotPlayer Movement Component is null!"));
	}
	
}

void ARobotPlayerCharacter::SprintReleased()
{
	if (RobotPlayerMovementComponent)
	{
		RobotPlayerMovementComponent->SprintReleased();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("RobotPlayer Movement Component is null!"));
	}
	
}

// Called every frame
void ARobotPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsStepping)
	{
		AnimationSteps();
	}
}

// Called to bind functionality to input
void ARobotPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::Look);

		// Equip Weapon
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::Equip);

		// Light Attack
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::LightAttack);

		// Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::HeavyAttack);

		// Slide
		EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::Slide);

		// Sprint Pressed
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::SprintPressed);

		// Sprint Released
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARobotPlayerCharacter::SprintReleased);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ARobotPlayerCharacter::PlayLightAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	bIsStepping = true;
	bImmobile = true;
	if (AnimInstance && LightAttackMontage)
	{
		AnimInstance->Montage_Play(LightAttackMontage);
	}
}



void ARobotPlayerCharacter::DisableMovementWhenAction()
{
}

void ARobotPlayerCharacter::SetDefaultApparel()
{
	// Set up Apparel
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);

	if (SwordMesh != nullptr)
	{
		Sword->SetSkeletalMesh(SwordMesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Sword Mesh is missing!"));
	}

	if (Cloth != nullptr)
	{
		Cloth->SetSkeletalMesh(ClothMesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Cloth Mesh is missing!"));
	}

	if (HatMesh != nullptr)
	{
		Hat->SetStaticMesh(HatMesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Hat Mesh is missing!"));
	}

	if (ShieldFrontMesh != nullptr)
	{
		ShieldFront->SetSkeletalMesh(ShieldFrontMesh);
		ShieldBack->SetSkeletalMesh(ShieldFrontMesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Shield Front Mesh is missing!"));
	}

	if (ShieldSideMesh != nullptr)
	{
		ShieldLeft->SetSkeletalMesh(ShieldSideMesh);
		ShieldRight->SetSkeletalMesh(ShieldSideMesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Shield Size Mesh is missing!"));
	}
}

void ARobotPlayerCharacter::AnimationSteps()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		CurrStep = AnimInstance->GetCurveValue(FName("StepCurve"));
		float offset = CurrStep - PrevStep;

		StepForward(offset * StepDistanceModifier);
		PrevStep = CurrStep;
	}
}

void ARobotPlayerCharacter::StepForward(float StepDistance)
{
	AddMovementInput(GetActorForwardVector(), StepDistance);
}

/// <summary>
/// Clearn up Animation State
/// </summary>
void ARobotPlayerCharacter::EndAttackAnimation()
{
	bIsStepping = false;
	bImmobile = false;
	PrevStep = 0.f;
	CurrStep = 0.f;
	ActionState = EActionState::EAS_Unoccupied;
}

