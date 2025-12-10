// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyControllerBase.generated.h"

class UAIReactionData;
class AEnemy;
class UStateTreeAIComponent;
class UAttackDefinitionData;

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
	AEnemyControllerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
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
	FVector GetBackwardDirectionFromTarget();

	UFUNCTION(BlueprintCallable)
	void EnterIdleMode();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat");
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat");
	const UAttackDefinitionData* CurrAttackData;

	/** Used for State Tree AtkChooser Cooldown **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat");
	float CurrAttackCd = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat");
	float AttackCooldownDuration = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat");
	bool CanAttack = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="AI")
	UStateTreeAIComponent* StateTreeComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="AI")
	UAIReactionData* ReactionData;

	void StopMovement();

	UFUNCTION(BlueprintCallable)
	bool IsImmoblie();

	UFUNCTION(BlueprintCallable)
	bool IsStrafing();

	UFUNCTION(BlueprintCallable)
	void OnDeath();

	UFUNCTION(BlueprintCallable)
	void SetAllowStrafingRotation(bool bAllow);

	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnMoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	UFUNCTION(BlueprintCallable)
	void ResetAttackCd();

	UFUNCTION(BlueprintCallable)
	void SetCurrAttackData(const UAttackDefinitionData* Data);

	UFUNCTION(BlueprintCallable)
	void SetCanAttack(bool on);

	void HandleAttackCooldownFinished();
private:
	AEnemy* EnemyPawn;
	FTimerHandle AttackCooldownTimer;
	EAIState AIState = EAIState::EAS_Unoccupied;
};
