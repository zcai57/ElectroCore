// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Interface/IEnemyAI.h"
#include "Enemy.generated.h"

UCLASS()
class PROJECTROBOT_API AEnemy : public ACharacter, public IIEnemyAI
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void UpdateWalkSpeed(float NewSpeed);


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetDesiredAttackAngle() const PURE_VIRTUAL(AEnemyCharacter::GetDesiredAttackAngle, return FVector::ZeroVector;);

};
