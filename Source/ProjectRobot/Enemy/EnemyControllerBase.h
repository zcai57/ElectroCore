// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyControllerBase.generated.h"

UENUM(BlueprintType)
enum class EAIState : uint8
{
	EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMoveRequestFinished, bool, bSuccess);
/**
 * 
 */
UCLASS()
class PROJECTROBOT_API AEnemyControllerBase : public AAIController
{
	GENERATED_BODY()
public:
	virtual void Tick(float DeltaTime) override;

	void OnPossess(APawn* InPawn) override;
	
	void FaceCombatTarget();
	
	UPROPERTY(BlueprintAssignable, Category="AI")
	FOnMoveRequestFinished OnMoveRequestFinished;

	UFUNCTION(BlueprintCallable)
	void StartMove(FVector TargetLocation, AActor* MyTarget = nullptr);

	UFUNCTION(BlueprintCallable)
	void EnterCombatMode();

	UFUNCTION(BlueprintCallable)
	void EnterIdleMode();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat");
	AActor* CombatTarget;

	void StopMovement();

	UFUNCTION(BlueprintCallable)
	bool IsImmoblie();

	UFUNCTION(BlueprintCallable)
	bool IsStrafing();
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);
	
private:
private:
	AEnemy* EnemyPawn;
	EAIState AIState = EAIState::EAS_Unoccupied;
};
