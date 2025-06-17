// Fill out your copyright notice in the Description page of Project Settings.


#include "../Character/RobotPlayerMovement.h"
#include "GameFramework/Character.h"

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
}

void URobotPlayerMovement::FSavedMove_Robot::PrepMoveFor(ACharacter* C)
{
    FSavedMove_Character::PrepMoveFor(C);
    URobotPlayerMovement* CharacterMovement = Cast<URobotPlayerMovement>(C->GetMovementComponent());

    CharacterMovement->Safe_bWantsToSprint = Saved_bWantsToSprint;
}

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
}

URobotPlayerMovement::URobotPlayerMovement()
{
}

void URobotPlayerMovement::SprintPressed()
{
    Safe_bWantsToSprint = true;
}

void URobotPlayerMovement::SprintReleased()
{
    Safe_bWantsToSprint = false;
}
