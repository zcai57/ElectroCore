// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "RobotCharacterState.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "RobotPlayerCharacter.generated.h"

class UAttackComponent;
class AWeaponBase;
struct FInputActionValue;
struct FOnAttributeChangeData;
// struct FGameplayTagContainer;
struct FGameplayTag;
class UGameplayEffect;
class URobotAbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UGameplayAbility;
class URobotPlayerMovement;
class UAbilitySystemComponent;
class UStartingAttributeSet;
class UMotionWarpingComponent;
class AJet;

UCLASS()
class PROJECTROBOT_API ARobotPlayerCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	class URobotPlayerMovement* RobotPlayerMovementComponent;
private:
	/** Motion Warping component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motion Warping", meta = (AllowPrivateAccess = "true"))
	UMotionWarpingComponent* MotionWarpingComp;
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float CameraArmLength = 250.f;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	float LookInterpSpeed = 10.f;

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
	UInputAction* DodgeAction;

	/** Light Attack Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LightAttackAction;

	/** Light Attack Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* HeavyAttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AdvanceStateAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BlockAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FocusAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	UStaticMesh* HatMesh;

	// UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	// USkeletalMesh* SwordMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	UChildActorComponent* WeaponChild;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* ShieldSideMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* ShieldFrontMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Apparel", meta = (AllowPrivateAccess = "true"))
	USkeletalMesh* ClothMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	float StepDistanceModifier = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	UAttackComponent* AttackComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer LightComboTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer HeavyAttackTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer DodgeTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer BlockTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer AdvanceTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer DodgeAttackTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer BlockAttackTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	FGameplayTagContainer FocusTag;

	UPROPERTY(EditDefaultsOnly, Category = "Attributes")
	TObjectPtr<UDataTable> DT_StartingAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning|Camera", meta = (AllowPrivateAccess = "true"))
	FVector CameraSocketOffset = FVector(0.0f, 50.f, 20.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning|Camera", meta = (AllowPrivateAccess = "true"))
	float OffsetInterpSpeed = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tuning|Camera", meta = (AllowPrivateAccess = "true"))
	float FocusInterpSpeed = 3.0f;

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

	UFUNCTION(BlueprintCallable)
	void SetFocusTarget(AActor* FocusedTarget);

	UFUNCTION(BlueprintCallable)
	AActor* GetFocusedTarget();

	UFUNCTION(BlueprintCallable)
	void ClampMotionWarpDist(float dist);

	UFUNCTION(BlueprintCallable)
	void StartCameraMove(FVector Offset = FVector::ZeroVector, float LengthOffset = 0.f, float BlendInTime = -1.f, float BlendOutTime = -1.f);

	UFUNCTION(BlueprintCallable)
	void StopCameraMove();

	UFUNCTION(BlueprintCallable)
	void NotifyCameraMove(float DeltaSeconds);

	void SetInvulnerability(bool Invulnerable);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GAS", meta = (AllowPrivateAccess = "true"))
	const UStartingAttributeSet* StartAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Attributes")
	TSubclassOf<UGameplayEffect> StartingAttributeEffect;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	void Equip(const FInputActionValue& Value);

	void LightAttack(const FInputActionValue& Value);

	void HeavyAttack(const FInputActionValue& Value);

	virtual void Crouch(bool bClientSimulation = false) override;

	void Focus();

	void Slide();

	void Dodge();

	void HandleAbilityAction1();

	void AdvanceState();

	void Block();

	void SprintPressed();

	void SprintReleased();

	void Jump();

	void StopJumping();

	void Death();
	
	void BindAttributeDelegate();
	
	void OnEnergyChanged(const FOnAttributeChangeData& Data);
	
	UFUNCTION() void Input_CrouchStarted();

	void OnSteppingTagChanged(FGameplayTag, int32 NewCount);

	void OnImmobileTagChanged(FGameplayTag, int32 NewCount);

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

	void AddCharacterAbilities();

	FCollisionQueryParams GetIgnoreCharacterParams() const;
private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState;

	UPROPERTY(EditAnywhere, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	/*UPROPERTY(VisibleAnywhere)
	ECharacterState CharacterState;*/

	/*UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EAnimState AnimState;

	UPROPERTY(VisibleAnywhere)
	ECharacterDirection CharacterDirection;*/

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	TObjectPtr<UAnimMontage> LightAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Transforms)
	TSubclassOf<AJet> JetBpClass;

	TObjectPtr<AActor> FocusedActor = nullptr;

	UPROPERTY(VisibleAnywhere)
	bool bImmobile = false;

	float CameraLookSensitivity = 1.0f;
	float RotationSpeed = 400.f;
	// States
	uint8 LightAttackStage = 0;
	bool bIsStepping = false;
	float PrevStep = 0.f;
	float CurrStep = 0.f;
	bool bIsFocused = false;

	// Camera Notify
	float NotifyBlendInSpeed = 8.f;
	float NotifyBlendOutSpeed = 8.f;
	FVector NotifyCameraOffset = FVector::ZeroVector;
	float NotifyLengthOffset = 0.f;
	bool bCameraNotifyStart = false;
	float NotifyAlpha = 0.f;
	float BoomBlendSpeed = 12.f;

	// Focus mode
	FVector FocusOffset = FVector::ZeroVector;      // e.g., your “focus” sideways push
	float   FocusArmAdd = 0.f;

	// Private Functions
	void PlayLightAttackMontage();

	void DisableMovementWhenAction();

	void SetDefaultApparel();

	void AnimationSteps();

};
