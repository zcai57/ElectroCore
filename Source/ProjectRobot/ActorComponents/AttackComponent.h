// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProjectRobot/Data/AttackTraceData.h"
#include "ProjectRobot/Data/TraceWindow.h"
#include "AttackComponent.generated.h"

class AWeaponBase;
// struct FTraceWindow;
class UAttackData;

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	EAS_Idle UMETA(DisplayName = "Idle"),
	EAS_Attacking UMETA(DisplayName = "Attacking"),
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTROBOT_API UAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttackComponent();

	UFUNCTION(BlueprintCallable)
	void StartDamageTrace(FName Key, UAttackTraceData* TraceData);

	UFUNCTION(BlueprintCallable)
	void StartCounterTrace(const FTraceWindow& TraceData);

	UFUNCTION(BlueprintCallable)
	void EndDamageTrace(UAttackTraceData* TraceData);

	UFUNCTION(BlueprintCallable)
	void EndCounterTrace();

	// UFUNCTION(BlueprintCallable)
	// void DoDamageTrace(const FName& Key, const FTraceWindow& TraceData);

	void BindWeapon(AWeaponBase* Weapon);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Debug")
	bool bEnableDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Attack")
	float LightAtkKnockback = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning|Attack")
	float HeavyAtkKnockback = 1000.f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void DoCounterTrace();

	void EnableWeaponHitbox();

	void DisableWeaponHitbox();

	UFUNCTION()
	void WeaponHitRegister(UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void HandleHitReactionAndHitStop(FGameplayEventData& Payload);

	void SendHitGameplayCue( AActor* TargetActor,
	UPrimitiveComponent* TargetComp);

	void ComputeImpact(FVector& ImpactPoint, FVector& ImpactNormal, AActor* TargetActor, UPrimitiveComponent* TargetComp);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Keeps track of which actors you’ve already hit for each trace window (by Key).
	TSet<TWeakObjectPtr<AActor>> HitActors;
	// Stores all currently active trace windows (attack hitboxes).
	// You can have multiple attacks or multiple sockets tracing simultaneously.
	// UPROPERTY() mutable TMap<FName, UAttackTraceData*> TraceDataSet;

	// Current Counter TraceData
	FTraceWindow ActiveCounterTraceData;
	// Current Counter Trace State
	bool bCounterTraceActive = false;

	// Weapon
	AWeaponBase* boundWeapon;

	FCollisionQueryParams GetIgnoreCharacterParams() const;
	UAttackTraceData* currData;
	AActor* currOwner;
};

