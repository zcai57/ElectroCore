// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyControllerBase.h"

#include "Enemy.h"
#include "Navigation/PathFollowingComponent.h"

void AEnemyControllerBase::StartMove(FVector TargetLocation, AActor* MyTarget)
{
	if(IsImmoblie())
	{
		OnMoveRequestFinished.Broadcast(false);
		return;
	}

	EPathFollowingRequestResult::Type Result;
	if (MyTarget == nullptr)
	{
		Result = MoveToLocation(TargetLocation);
	} else
	{
		Result = MoveToActor(MyTarget);
	}

	UE_LOG(LogTemp, Warning, TEXT("%s: MoveTo returned %s"),
	   *GetName(),
	   *UEnum::GetValueAsString(Result));

	// --- Handle cases where Unreal won't fire ReceiveMoveCompleted ---
	if (Result == EPathFollowingRequestResult::Failed)
	{
		// No path at all (bad navmesh or invalid goal)
		OnMoveRequestFinished.Broadcast(false);
	}
	else if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		// AI is already within acceptance radius — no OnMoveCompleted event will be fired
		OnMoveRequestFinished.Broadcast(true);
	}
}

void AEnemyControllerBase::EnterCombatMode()
{
	AIState = EAIState::EAS_Attacking;
	if (EnemyPawn)
	{
		EnemyPawn->SetStrafingMovement(true);
		SetFocus(CombatTarget, EAIFocusPriority::Gameplay);
	}
}

void AEnemyControllerBase::EnterIdleMode()
{
	AIState = EAIState::EAS_Unoccupied;
	if (EnemyPawn)
	{
		EnemyPawn->SetStrafingMovement(false);
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void AEnemyControllerBase::BeginPlay()
{
	Super::BeginPlay();
}


void AEnemyControllerBase::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	EnemyPawn = Cast<AEnemy>(InPawn);

	ReceiveMoveCompleted.RemoveDynamic(this, &AEnemyControllerBase::OnMoveCompleted);
	ReceiveMoveCompleted.AddDynamic(this, &AEnemyControllerBase::OnMoveCompleted);
}

void AEnemyControllerBase::OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	Super::OnMoveCompleted(RequestID, Result);
	OnMoveRequestFinished.Broadcast(Result == EPathFollowingResult::Type::Success);
}


void AEnemyControllerBase::StopMovement()
{
	Super::StopMovement();
	OnMoveRequestFinished.Broadcast(false);
}

bool AEnemyControllerBase::IsImmoblie()
{
	if (EnemyPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("IsImmobile: %s"), EnemyPawn->IsImmobile() ? TEXT("true") : TEXT("false"));
		return EnemyPawn->IsImmobile();
	}
	return false;
}

bool AEnemyControllerBase::IsStrafing()
{
	if (EnemyPawn)
	{
		return EnemyPawn->IsStrafing();
	}
	return false;
}

void AEnemyControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AEnemyControllerBase::FaceCombatTarget()
{
	if (!CombatTarget || !GetPawn())
		return;

	if (!bAllowStrafe) bAllowStrafe = true;

	// Compute the direction vector from pawn to target
	const FVector Direction = CombatTarget->GetActorLocation() - GetPawn()->GetActorLocation();

	// Convert that direction into a rotation
	FRotator TargetRotation = Direction.Rotation();

	// Apply only the yaw (horizontal rotation), ignore pitch/roll
	TargetRotation.Pitch = 0.f;
	TargetRotation.Roll = 0.f;

	
	// Set the controller rotation — this drives ControlRotation
	SetControlRotation(TargetRotation);
	// GetPawn()->SetActorRotation(TargetRotation);
}
