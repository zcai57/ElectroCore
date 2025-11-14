// Fill out your copyright notice in the Description page of Project Settings.
#include "RobotPlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "ProjectRobot/AttributeSet/StartingAttributeSet.h"
#include "ProjectRobot/GAS/RobotAbilitySystemComponent.h"
#include "ProjectRobot/Weapon/WeaponBase.h"
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
#include "MotionWarpingComponent.h"
#include "../Vehicle/Jet.h"
#include "ProjectRobot/ActorComponents/AttackComponent.h"
#include "ProjectRobot/Weapon/WeaponBase.h"

#define DEBUG_MOVEMENT 0

// Sets default values
ARobotPlayerCharacter::ARobotPlayerCharacter(const FObjectInitializer& ObjectInitializer) 
: Super(ObjectInitializer.SetDefaultSubobjectClass<URobotPlayerMovement>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Set collision for attacks
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);

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
	CameraBoom->TargetArmLength = CameraArmLength; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Set up Apparel
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	// Sword = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RobotSwordMesh"));
	// Sword->SetupAttachment(GetMesh(), FName("RightHandSocket"));

	WeaponChild = CreateDefaultSubobject<UChildActorComponent>(TEXT("WeaponChild"));
	WeaponChild->SetupAttachment(GetMesh(), FName("RightHandSocket"));
	
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
	
	// Setup GAS
	AbilitySystemComponent = CreateDefaultSubobject<URobotAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// Motion Warping
	MotionWarpingComp = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComp"));

	// Attack Component
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
}

/// <summary>
/// 进入FocusTarget模式R
/// Call from Focus GameplayAbility
/// </summary>
/// <param name="FocusedTarget"></param>
void ARobotPlayerCharacter::SetFocusTarget(AActor* FocusedTarget)
{
	check(FocusedTarget);
	
	RobotPlayerMovementComponent->bOrientRotationToMovement = false;
	RobotPlayerMovementComponent->bUseControllerDesiredRotation = true;

	bIsFocused = true;
	FocusedActor = FocusedTarget;
}



// Called when the game starts or when spawned
void ARobotPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	// Init Movement Component
	RobotPlayerMovementComponent = Cast<URobotPlayerMovement>(GetCharacterMovement());
	// Set up Apparel items
	SetDefaultApparel();
	//AbilitySystemComponent->InitAbilityActorInfo(this, this);
	// Start Abilities
	AddCharacterAbilities();
	// Set up Attribute

	AbilitySystemComponent->InitStats(UStartingAttributeSet::StaticClass(), DT_StartingAttributes);
	if (IsValid(AbilitySystemComponent))
	{
		StartAttributeSet = AbilitySystemComponent->GetSet<UStartingAttributeSet>();
	}

	UE_LOG(LogTemp, Warning, TEXT("InitStats: %s"), *GetNameSafe(DT_StartingAttributes));
	if (StartAttributeSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("Heat=%f Energy=%f Status=%f"),
			StartAttributeSet->GetHeat(), StartAttributeSet->GetEnergy(), StartAttributeSet->GetStatus());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("StartAttributeSet is NULL"));
	}
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
		// Flatten rotation, zero out pitch and roll
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		// Align actor rotation to control rotation 
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


	// Rotate CameraBoom using the Look input (X for Yaw, Y for Pitch)
	FRotator NewRotation = Controller->GetControlRotation();
	if (!bIsFocused) NewRotation.Yaw += LookAxisVector.X * CameraLookSensitivity;
	NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch + LookAxisVector.Y * CameraLookSensitivity, -60.f, 60.f); // Limit pitch range

	//CameraBoom->SetRelativeRotation(NewRotation);
	const FRotator Rotation = Controller->GetControlRotation();
	Controller->SetControlRotation(NewRotation);
}

void ARobotPlayerCharacter::Equip(const FInputActionValue& Value)
{
}

void ARobotPlayerCharacter::LightAttack(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent || !LightComboTag.IsValid()) return;

	if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.DodgeWindowOpen")))
	{
		if (AbilitySystemComponent->TryActivateAbilitiesByTag(DodgeAttackTag))
		{
			UE_LOG(LogTemp, Log, TEXT("Triggered DodgeAttack while dodging."));
		}
	}
	else if (AbilitySystemComponent->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag("State.BlockWindowOpen")))
	{
		if (AbilitySystemComponent->TryActivateAbilitiesByTag(BlockAttackTag))
		{
			UE_LOG(LogTemp, Log, TEXT("Triggered BlockAttack while blocking."));
		}
	}
	else {
		AbilitySystemComponent->TryActivateAbilitiesByTag(LightComboTag);
		AbilitySystemComponent->AbilityLocalInputPressed(0);
	}
}

void ARobotPlayerCharacter::HeavyAttack(const FInputActionValue& Value)
{
	if (!AbilitySystemComponent || !HeavyAttackTag.IsValid()) return;
	AbilitySystemComponent->TryActivateAbilitiesByTag(HeavyAttackTag);

}

void ARobotPlayerCharacter::Crouch(bool bClientSimulation)
{
	/*Super::Crouch(bClientSimulation);*/

	if (RobotPlayerMovementComponent)
	{
		RobotPlayerMovementComponent->CrouchPressed();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("RobotPlayer Movement Component is null!"));
	}
}

void ARobotPlayerCharacter::Focus()
{
	// 不是Focus状态，进入Focus
	if (!bIsFocused)
	{
		if (!AbilitySystemComponent || !FocusTag.IsValid()) return;
		AbilitySystemComponent->TryActivateAbilitiesByTag(FocusTag);

	}
	else {
		if (!AbilitySystemComponent || !FocusTag.IsValid()) return;
		AbilitySystemComponent->CancelAbilities(&FocusTag);
		// 改变行动模式
		RobotPlayerMovementComponent->bOrientRotationToMovement = true;
		RobotPlayerMovementComponent->bUseControllerDesiredRotation = false;
		// Reset状态
		FocusedActor = nullptr;
		bIsFocused = false;
	}
}

void ARobotPlayerCharacter::Slide()
{

}

void ARobotPlayerCharacter::Dodge()
{
	if (!AbilitySystemComponent || !DodgeTag.IsValid()) return;
	AbilitySystemComponent->TryActivateAbilitiesByTag(DodgeTag);
}

void ARobotPlayerCharacter::HandleAbilityAction1()
{
	UE_LOG(LogTemp, Warning, TEXT("Handle Ability 1"));
	AJet* JetActor;
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (JetBpClass)
	{
		this->SetActorHiddenInGame(true);
		this->SetActorEnableCollision(false);
		JetActor = GetWorld()->SpawnActor<AJet>(JetBpClass, GetActorTransform());
		JetActor->Init(this);
		PC->Possess(JetActor);
	}
}

void ARobotPlayerCharacter::AdvanceState()
{
	if (!AbilitySystemComponent || !AdvanceTag.IsValid()) return;
	AbilitySystemComponent->TryActivateAbilitiesByTag(AdvanceTag);
}

void ARobotPlayerCharacter::Block()
{
	if (!AbilitySystemComponent || !BlockTag.IsValid()) return;
	AbilitySystemComponent->TryActivateAbilitiesByTag(BlockTag);
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

void ARobotPlayerCharacter::Jump()
{
	// Boosting
	if (RobotPlayerMovementComponent->IsFalling() && !RobotPlayerMovementComponent->bWantsToBoost)
	{
		RobotPlayerMovementComponent->BoostPressed();
	}
	else {
		ACharacter::Jump();
	}
}

void ARobotPlayerCharacter::StopJumping()
{
	if(RobotPlayerMovementComponent->bWantsToBoost)
	{
		RobotPlayerMovementComponent->BoostReleased();
	}
	ACharacter::StopJumping();	
}

void ARobotPlayerCharacter::Input_CrouchStarted()
{
	Crouch();
}

void ARobotPlayerCharacter::OnSteppingTagChanged(FGameplayTag, int32 NewCount)
{
	bIsStepping = (NewCount > 0);
}

void ARobotPlayerCharacter::OnImmobileTagChanged(FGameplayTag, int32 NewCount)
{
	bImmobile = (NewCount > 0);
}


// Called every frame
void ARobotPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsStepping)
	{
		AnimationSteps();
	}
#if DEBUG_MOVEMENT == 1
	FString VelocityString = RobotPlayerMovementComponent->Velocity.ToString();
	GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("Velocity: %s"), *VelocityString));

#endif // DEBUGMOVEMENT

	// Set ControlRotation to direciton of focusedActor
	if (bIsFocused && FocusedActor)
	{
		FVector ToTarget = FocusedActor->GetActorLocation() - GetActorLocation();
		FRotator TargetRot = ToTarget.Rotation();
		APlayerController* PlayerController = Cast<APlayerController>(GetController());
		if (PlayerController)
		{
			FRotator CharacterRot = PlayerController->GetControlRotation();
			// Ignore Pitch
			TargetRot.Pitch = CharacterRot.Pitch;
			PlayerController->SetControlRotation(FMath::RInterpTo(CharacterRot, TargetRot, DeltaTime, FocusInterpSpeed));
		}
		CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, CameraSocketOffset, DeltaTime, OffsetInterpSpeed);
	}
	else {
		CameraBoom->SocketOffset = FMath::VInterpTo(CameraBoom->SocketOffset, FVector::ZeroVector, DeltaTime, OffsetInterpSpeed);
	}

	// Notify Camera Movement
	NotifyCameraMove(DeltaTime);
}

// Called to bind functionality to input
void ARobotPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ARobotPlayerCharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::Look);

		// Equip Weapon
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::Equip);

		// Light Attack
		EnhancedInputComponent->BindAction(LightAttackAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::LightAttack);

		// Heavy Attack
		EnhancedInputComponent->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::HeavyAttack);

		// Crouch Pressed
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::Input_CrouchStarted);

		// Shift Pressed
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::Dodge);
		
		// Slide
		/*EnhancedInputComponent->BindAction(SlideAction, ETriggerEvent::Triggered, this, &ARobotPlayerCharacter::Slide);*/

		// Sprint Pressed
		/*EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::SprintPressed);*/

		// Sprint Released
		//EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ARobotPlayerCharacter::SprintReleased);

		// Ability1 Pressed
		EnhancedInputComponent->BindAction(AdvanceStateAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::AdvanceState);

		// Ability2 Presed
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::Block);

		// Focus Action
		EnhancedInputComponent->BindAction(FocusAction, ETriggerEvent::Started, this, &ARobotPlayerCharacter::Focus);
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


	AWeaponBase* Weapon = Cast<AWeaponBase>(WeaponChild->GetChildActor());
	if (Weapon)
	{
		Weapon->SetOwner(this);
		Weapon->BindToOwnerAttackComponent(AttackComponent);
	}
	// if (SwordMesh != nullptr)
	// {
	// 	Sword->SetSkeletalMesh(SwordMesh);
	// }
	// else {
	// 	UE_LOG(LogTemp, Warning, TEXT("Sword Mesh is missing!"));
	// }

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

void ARobotPlayerCharacter::AddCharacterAbilities()
{
	URobotAbilitySystemComponent* RobotASC = CastChecked<URobotAbilitySystemComponent>(AbilitySystemComponent);

	/*if (!HasAuthority()) return;*/

	RobotASC->InitAbilityActorInfo(this, this);
	RobotASC->AddCharacterAbilities(StartupAbilities);

	RobotASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("State.Stepping"), EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ARobotPlayerCharacter::OnSteppingTagChanged);
	RobotASC->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag("State.Immobile"), EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ARobotPlayerCharacter::OnImmobileTagChanged);
}

FCollisionQueryParams ARobotPlayerCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

AActor* ARobotPlayerCharacter::GetFocusedTarget()
{
	if (!FocusedActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("FocusedActor is nullptr"));
	}
	return FocusedActor;
}

/// Clamp motionwarp distance with dist if no target is locked.
/// If valid target exist, clamp the max warp distance to dist.
/// @param dist 
void ARobotPlayerCharacter::ClampMotionWarpDist(float dist)
{
	if (!MotionWarpingComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("MotionWarpingComp is null on %s"), *GetName());
		return;
	}

	FVector Target = FVector::ZeroVector;
	FVector Direction = FVector::ZeroVector;
	FVector SelfLoc = GetActorLocation();
	if (FocusedActor)
	{
		FVector ActorLoc = FocusedActor->GetActorLocation();
		Target = FocusedActor->GetActorLocation();

		Direction = ActorLoc - SelfLoc;
		float Distance = Direction.Size();

		if (Distance > dist)
		{
			Direction.Normalize();
			Target = dist * Direction + SelfLoc;
		}
	}
	else {
		// Use controller yaw when no target
		const float Yaw = Controller ? Controller->GetControlRotation().Yaw : GetActorRotation().Yaw;
		Direction = FRotator(0.f, Yaw, 0.f).Vector();

		Target = SelfLoc + Direction * dist;
	}

	if (Target != FVector::ZeroVector)
	{
		MotionWarpingComp->AddOrUpdateWarpTargetFromLocation("AttackTarget", Target);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("MotionWarp Target not Valid"));
	}
}

void ARobotPlayerCharacter::StartCameraMove(FVector Offset, float LengthOffset, float BlendInTime, float BlendOutTime)
{
	if (Offset != FVector::ZeroVector) NotifyCameraOffset = Offset;
	if (BlendInTime > 0.f) NotifyBlendInSpeed = BlendInTime;
	if (BlendOutTime > 0.f) NotifyBlendOutSpeed = BlendOutTime;
	if (LengthOffset > 0.f) NotifyLengthOffset = LengthOffset;
	bCameraNotifyStart = true;
}

void ARobotPlayerCharacter::StopCameraMove()
{
	bCameraNotifyStart = false;
}

void ARobotPlayerCharacter::NotifyCameraMove(float DeltaSeconds)
{
	if (!CameraBoom) return;

	const float TargetAlpha = bCameraNotifyStart ? 1.f : 0.f;
	const float AlphaSpeed = bCameraNotifyStart ? NotifyBlendInSpeed : NotifyBlendOutSpeed;

	NotifyAlpha = FMath::FInterpTo(NotifyAlpha, TargetAlpha, DeltaSeconds, AlphaSpeed);
	// build desired arm length
	const FVector DesiredSocketOffset =
		CameraSocketOffset
		+ FocusOffset
		+ (NotifyCameraOffset * NotifyAlpha);

	const float DesiredArmLength =
		CameraArmLength
		+ FocusArmAdd
		+ (NotifyLengthOffset * NotifyAlpha);

	CameraBoom->SocketOffset = FMath::VInterpTo(
		CameraBoom->SocketOffset, DesiredSocketOffset, DeltaSeconds, -1.f);

	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength, DesiredArmLength, DeltaSeconds, -1.f);
}

void ARobotPlayerCharacter::SetInvulnerability(bool Invulnerable)
{
	if (Invulnerable)
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	} else
	{
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
