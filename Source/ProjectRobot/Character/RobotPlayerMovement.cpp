// Fill out your copyright notice in the Description page of Project Settings.


#include "../Character/RobotPlayerMovement.h"
#include "RobotPlayerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"

#pragma region Saved Move
bool URobotPlayerMovement::FSavedMove_Robot::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
    FSavedMove_Robot* NewRobotMove = static_cast<FSavedMove_Robot*>(NewMove.Get());
    
    if (Saved_bWantsToSprint != NewRobotMove->Saved_bWantsToSprint)
    {
        return false;
    }
    return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void URobotPlayerMovement::FSavedMove_Robot::Clear()
{
    FSavedMove_Character::Clear();
    Saved_bWantsToSprint = 0;
    Saved_bPrevWantsToCrouch = 0;
}

uint8 URobotPlayerMovement::FSavedMove_Robot::GetCompressedFlags() const
{
    uint8 Result = FSavedMove_Character::GetCompressedFlags();
    if (Saved_bWantsToSprint) Result |= FLAG_Custom_0;

    return Result;
}

void URobotPlayerMovement::FSavedMove_Robot::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
    FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
    
    URobotPlayerMovement* CharacterMovement = Cast<URobotPlayerMovement>(C->GetMovementComponent());

    Saved_bWantsToSprint = CharacterMovement->Safe_bWantsToSprint;
    Saved_bPrevWantsToCrouch = CharacterMovement->Safe_bPrevWantsToCrouch;
}

void URobotPlayerMovement::FSavedMove_Robot::PrepMoveFor(ACharacter* C)
{
    FSavedMove_Character::PrepMoveFor(C);
    URobotPlayerMovement* CharacterMovement = Cast<URobotPlayerMovement>(C->GetMovementComponent());

    CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
    CharacterMovement->Safe_bPrevWantsToCrouch = Saved_bPrevWantsToCrouch;
}

#pragma endregion

#pragma region Network Prediction Data
URobotPlayerMovement::FNetworkPredictionData_Client_Robot::FNetworkPredictionData_Client_Robot(const UCharacterMovementComponent& clientMovement) : Super(clientMovement)
{
}

FSavedMovePtr URobotPlayerMovement::FNetworkPredictionData_Client_Robot::AllocateNewMove()
{
    return FSavedMovePtr(new FSavedMove_Robot);
}

FNetworkPredictionData_Client* URobotPlayerMovement::GetPredictionData_Client() const
{
    check(PawnOwner != nullptr);

    if (ClientPredictionData == nullptr)
    {
        URobotPlayerMovement* MutableThis = const_cast<URobotPlayerMovement*>(this);
        MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Robot(*this);
        MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
        MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
    }
    return ClientPredictionData;
}
#pragma endregion

#pragma region CMC

void URobotPlayerMovement::InitializeComponent()
{
    Super::InitializeComponent();

    RobotCharacterOwner = Cast<ARobotPlayerCharacter>(GetOwner());
}

void URobotPlayerMovement::UpdateFromCompressedFlags(uint8 Flags)
{
    Super::UpdateFromCompressedFlags(Flags);

    Safe_bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void URobotPlayerMovement::OnMovementUpdated(float deltaSecond, const FVector& OldLocation, const FVector& OldVelocity)
{
    Super::OnMovementUpdated(deltaSecond, OldLocation, OldVelocity);

    if (MovementMode == MOVE_Walking)
    {
        if (Safe_bWantsToSprint)
        {
            MaxWalkSpeed = Sprint_MaxWalkSpeed;
        }
        else {
            MaxWalkSpeed = Walk_MaxWalkSpeed;
        }
    } 

    Safe_bPrevWantsToCrouch = bWantsToCrouch;
}

bool URobotPlayerMovement::IsMovingOnGround() const
{
    return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool URobotPlayerMovement::CanCrouchInCurrentState() const
{
    return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

void URobotPlayerMovement::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
    // Press crouch enter crouch
    if (MovementMode == MOVE_Walking && Safe_bPrevWantsToCrouch)
    {
        FHitResult PotentialSlideSurface;
        UE_LOG(LogTemp, Warning, TEXT("Enough Vel: %s"), (Velocity.SizeSquared() > pow(Slide_EnterSpeed, 2)) ? TEXT("true") : TEXT("false"));
        UE_LOG(LogTemp, Warning, TEXT("Slidable Surface: %s"), GetSlideSurface(PotentialSlideSurface) ? TEXT("true") : TEXT("false"));
        if (Velocity.SizeSquared() > pow(Slide_EnterSpeed, 2) && GetSlideSurface(PotentialSlideSurface))
        {
            EnterSlide();
        }

    }
    // Press crouch when sliding, exist slide

    if (IsCustomMovementMode(CMOVE_Slide) && !bWantsToCrouch)
    {
        ExitSlide();
    }

    Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void URobotPlayerMovement::PhysCustom(float deltaTime, int32 Iterations)
{
    Super::PhysCustom(deltaTime, Iterations);

    switch (CustomMovementMode)
    {
    case CMOVE_Slide:
        PhysSlide(deltaTime, Iterations);
        break;
    case CMOVE_Boost:
        PhysBoost(deltaTime, Iterations);
        break;
    default:
        UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"));
    }
}

void URobotPlayerMovement::EnterSlide()
{
    // Press crouch second time will set it to false, to keep state, set to true
    bWantsToCrouch = true;
    Velocity += Velocity.GetSafeNormal2D() * Slide_EnterImpulse;
    SetMovementMode(MOVE_Custom, CMOVE_Slide);
}

void URobotPlayerMovement::ExitSlide()
{
    bWantsToCrouch = false;

    FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
    FHitResult Hit;
    SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
    SetMovementMode(MOVE_Walking);
}

;void URobotPlayerMovement::PhysSlide(float deltaTime, int32 Iterations)
{
    if (deltaTime < MIN_TICK_TIME) {
        return;
    }
    
    FHitResult SurfaceHit;
    // Check Vel and valid surface before update
    if (!GetSlideSurface(SurfaceHit) || Velocity.SizeSquared() < pow(Slide_ExitSpeed, 2))
    {
        ExitSlide();
        StartNewPhysics(deltaTime, Iterations);
        return;
    }

    // Surface Gravity
    Velocity += Slide_GravityForce * FVector::DownVector * deltaTime; // v += a * dt;

    // Strafe
    if (FMath::Abs(FVector::DotProduct(Acceleration.GetSafeNormal(), UpdatedComponent->GetRightVector())) > .5) // Acceleration: input vector
    {
        Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector());
    }
    else {
        Acceleration = FVector::ZeroVector;
    }

    // Calc Velocity
    if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        CalcVelocity(deltaTime, Slide_Friction, true, GetMaxBrakingDeceleration());
    }
    ApplyRootMotionToVelocity(deltaTime);

    // Perform Move
    ++Iterations;
    bJustTeleported = false;

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
    FHitResult Hit(1.f);
    FVector Adjusted = Velocity * deltaTime; // x = v * dt
    FVector VelPlanetDir = FVector::VectorPlaneProject(Velocity, SurfaceHit.Normal).GetSafeNormal();
    FQuat NewRotation = FRotationMatrix::MakeFromXZ(VelPlanetDir, SurfaceHit.Normal).ToQuat();
    SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

    if (Hit.Time < 1.f)
    {
        HandleImpact(Hit, deltaTime, Adjusted);
        SlideAlongSurface(Adjusted, (1.f - Hit.Time), Hit.Normal, Hit, true);
    }

    // Check Vel and valid surface before update
    FHitResult NewSurfaceHit;
    if (!GetSlideSurface(NewSurfaceHit) || Velocity.SizeSquared() < pow(Slide_ExitSpeed, 2))
    {
        ExitSlide();
    }

    // Update Outgoing Velocity & Acceleration
    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
    }
}

bool URobotPlayerMovement::GetSlideSurface(FHitResult& Hit) const
{
    FVector Start = UpdatedComponent->GetComponentLocation();
    FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f * FVector::DownVector;
    FName ProfileName = TEXT("BlockAll");
    return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, RobotCharacterOwner->GetIgnoreCharacterParams());
}

void URobotPlayerMovement::EnterBoost()
{
    const FVector Forward = UpdatedComponent->GetForwardVector().GetSafeNormal();
    const float ExistingForwardSpeed = FVector::DotProduct(Velocity, Forward);
    // Clamp forward momentum
    const float ClampedForwardSpeed = FMath::Min(ExistingForwardSpeed + Boost_ForwardImpulse, Boost_MaxForwardVelocity);
    // Update NewVelocity
    FVector NewVelocity = Velocity - (Forward * ExistingForwardSpeed);
    NewVelocity += Forward * Boost_ForwardImpulse + FVector::UpVector * Boost_UpImpulse;
    Velocity = NewVelocity;
   
    SetMovementMode(MOVE_Custom, CMOVE_Boost);
}

void URobotPlayerMovement::ExitBoost()
{
    FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
    FHitResult Hit;
    SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, true, Hit);
    SetMovementMode(MOVE_Walking);
}

void URobotPlayerMovement::PhysBoost(float deltaTime, int32 Iterations)
{
    if (deltaTime < MIN_TICK_TIME) {
        return;
    }

    FHitResult SurfaceHit;
    // Check Vel and valid surface before update
    if (GetBoostCeiling(SurfaceHit))
    {
        ExitBoost();
        StartNewPhysics(deltaTime, Iterations);
        return;
    }

    // Surface Gravity
    Velocity += Boost_GravityForce * FVector::DownVector * deltaTime; // v += a * dt;
    ApplyRootMotionToVelocity(deltaTime);

    // Perform Move
    ++Iterations;
    bJustTeleported = false;

    FVector OldLocation = UpdatedComponent->GetComponentLocation();
    FQuat OldRotation = UpdatedComponent->GetComponentRotation().Quaternion();
    FHitResult Hit(1.f);
    FVector Adjusted = Velocity * deltaTime; // x = v * dt
    FQuat NewRotation = FRotationMatrix::MakeFromXZ(UpdatedComponent->GetForwardVector().GetSafeNormal2D(), FVector::UpVector).ToQuat();
    SafeMoveUpdatedComponent(Adjusted, NewRotation, true, Hit);

    if (Hit.Time < 1.f)
    {
        HandleImpact(Hit, deltaTime, Adjusted);
    }

    //// Check Vel and valid surface before update
    FHitResult NewSurfaceHit;
    if (GetBoostCeiling(NewSurfaceHit) || Velocity.SizeSquared() < pow(Slide_ExitSpeed, 2))
    {       
        ExitBoost();
    }

    // Update Outgoing Velocity & Acceleration
    if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
    {
        Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / deltaTime;
    }
}

/// <summary>
/// Maybe not needed?
/// </summary>
/// <param name="Hit"></param>
/// <returns></returns>
bool URobotPlayerMovement::GetBoostCeiling(FHitResult& Hit) const
{
    FVector Start = UpdatedComponent->GetComponentLocation();
    FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.f * FVector::UpVector;
    FName ProfileName = TEXT("BlockAll");
    return GetWorld()->LineTraceSingleByProfile(Hit, Start, End, ProfileName, RobotCharacterOwner->GetIgnoreCharacterParams());
}

URobotPlayerMovement::URobotPlayerMovement()
{
    NavAgentProps.bCanCrouch = true;
}
#pragma endregion

#pragma region Input

void URobotPlayerMovement::SprintPressed()
{
    Safe_bWantsToSprint = true;
}

void URobotPlayerMovement::SprintReleased()
{
    Safe_bWantsToSprint = false;
}

void URobotPlayerMovement::BoostPressed()
{
    Safe_bWantsToBoost = true;
    bWantsToBoost = true;
    EnterBoost();
}

void URobotPlayerMovement::BoostReleased()
{
    Safe_bWantsToBoost = false;
    bWantsToBoost = false;
    ExitBoost();
}

void URobotPlayerMovement::CrouchPressed()
{
    bWantsToCrouch = !bWantsToCrouch;
}
bool URobotPlayerMovement::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
    return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}
#pragma endregion