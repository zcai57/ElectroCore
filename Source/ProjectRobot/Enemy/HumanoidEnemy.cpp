// Fill out your copyright notice in the Description page of Project Settings.


#include "HumanoidEnemy.h"

AHumanoidEnemy::AHumanoidEnemy()
{
}

void AHumanoidEnemy::BeginPlay()
{
	Super::BeginPlay();
}

void AHumanoidEnemy::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
}

FVector AHumanoidEnemy::GetDesiredAttackAngle() const
{
	return FVector::ZeroVector;
}

void AHumanoidEnemy::UpdateWalkSpeed(float NewSpeed)
{
	Super::UpdateWalkSpeed(NewSpeed);
}
