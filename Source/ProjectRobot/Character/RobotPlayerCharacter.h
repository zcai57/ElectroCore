// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "RobotCharacterState.h"
#include "RobotPlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAnimMontage;
class URobotPlayerMovement;

UCLASS()
class PROJECTROBOT_API ARobotPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	class URobotPlayerMovement* RobotPlayerMovementComponent;
private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Equip Weapon Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EquipAction;

	/** Sprint Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Light Attack Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	/** Light Attack Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* HatMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* SwordMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* ShieldSideMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* ShieldFrontMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* ClothMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float StepDistanceModifier = 100.f;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Sword;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Hat;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ShieldLeft;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ShieldRight;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ShieldFront;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* ShieldBack;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Cloth;


public:
	// Sets default values for this character's properties
	ARobotPlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Equip(const FInputActionValue& Value);

	void LightAttack(const FInputActionValue& Value);

	void HeavyAttack(const FInputActionValue& Value);

	void Crouch(const FInputActionValue& Value);

	void Slide(const FInputActionValue& Value);

	void SprintPressed();

	void SprintReleased();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns Custom Movement Controller */
	FORCEINLINE class URobotPlayerMovement* GetRobotPlayerMovement() const { return RobotPlayerMovementComponent; };
	/** Returns ECharacterState */
	/*UFUNCTION(BlueprintPure, Category = "Character")
	FORCEINLINE ECharacterState GetCharacterState() { return CharacterState; }*/
	/** ReTURNS ECharacterDirection */
	/*UFUNCTION(BlueprintPure, Category = "Character")
	FORCEINLINE ECharacterDirection GetCharacterDirection() { return CharacterDirection; }*/
	UFUNCTION(BlueprintCallable, Category = "Character")
	void StepForward(float StepDistance = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Character")
	void EndAttackAnimation();

	FCollisionQueryParams GetIgnoreCharacterParams() const;
private:
	float CameraLookSensitivity = 1.0f;
	float RotationSpeed = 400.f;

	/*UPROPERTY(VisibleAnywhere)
	ECharacterState CharacterState;*/

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState;

	/*UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EAnimState AnimState;

	UPROPERTY(VisibleAnywhere)
	ECharacterDirection CharacterDirection;*/

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> LightAttackMontage;

	UPROPERTY(VisibleAnywhere)
	bool bImmobile = false;


	// States
	uint8 LightAttackStage = 0;
	bool bIsStepping = false;
	float PrevStep = 0.f;
	float CurrStep = 0.f;

	// Private Functions
	void PlayLightAttackMontage();

	void DisableMovementWhenAction();

	void SetDefaultApparel();

	void AnimationSteps();
};
