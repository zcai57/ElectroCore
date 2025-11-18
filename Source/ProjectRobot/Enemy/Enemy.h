// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "ProjectRobot/Interface/IEnemyAI.h"
#include "Enemy.generated.h"


class UAttackComponent;
struct FGameplayTag;
struct FOnAttributeChangeData;
class UGameplayEffect;
class AEnemyControllerBase;
class UMotionWarpingComponent;
class URobotAbilitySystemComponent;
class UGameplayAbility;
class UStartingAttributeSet;
class UAbilitySystemComponent;

UCLASS()
class PROJECTROBOT_API AEnemy : public ACharacter, public IIEnemyAI, public IAbilitySystemInterface
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning", meta = (AllowPrivateAccess = "true"))
	FRotator RotationRate = FRotator(0.0f, 180.f, 0.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning", meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugDirection = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning", meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* WeaponChild;

public:
	// Sets default values for this character's properties
	AEnemy();
	
	void SetStrafingMovement(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void ClampMotionWarpDist(float dist);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** Motion Warping component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping", meta = (AllowPrivateAccess = "true"))
	UMotionWarpingComponent* MotionWarpingComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	UAttackComponent* AttackComponent;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UPROPERTY(EditAnywhere, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Attributes")
	TSubclassOf<UGameplayEffect> StartingAttributeEffect;

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TObjectPtr<UDataTable> DT_StartingAttributes;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	const class UStartingAttributeSet* StartAttributeSet;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void Death();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	virtual void UpdateWalkSpeed(float NewSpeed);

	void AddCharacterAbilities();

	void BindAttributeDelegate();
	
	void OnImmobileTagChanged(FGameplayTag, int32 NewCount);

	void DrawDebugDirection();
	
	void OnEnergyChanged(const FOnAttributeChangeData& Data);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual FVector GetDesiredAttackAngle() const PURE_VIRTUAL(AEnemyCharacter::GetDesiredAttackAngle, return FVector::ZeroVector;);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsImmobile();

	bool IsStrafing();
private:
	AEnemyControllerBase* EnemyController;
	bool bImmobile = false;
	bool bIsStrafing = false;
};
