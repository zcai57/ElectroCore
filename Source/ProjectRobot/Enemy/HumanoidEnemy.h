// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy.h"
#include "HumanoidEnemy.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTROBOT_API AHumanoidEnemy : public AEnemy
{
	GENERATED_BODY()
public:
	// Sets default values for this character's properties
	AHumanoidEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void UpdateWalkSpeed(float NewSpeed);


public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual FVector GetDesiredAttackAngle() const;

};
