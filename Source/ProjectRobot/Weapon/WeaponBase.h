// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

class UAttackComponent;
class UBoxComponent;

UCLASS()
class PROJECTROBOT_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponBase();

	UFUNCTION(BlueprintCallable)
	UBoxComponent* GetHitBox();

	void DebugDraw(bool on);

	void BindToOwnerAttackComponent(UAttackComponent* component);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDebugHitbox = false;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent* AttackHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* WeaponMesh;

	void DrawHitBox();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
