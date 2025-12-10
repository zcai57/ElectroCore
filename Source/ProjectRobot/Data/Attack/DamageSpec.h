#pragma once
#include "CoreMinimal.h"
#include "DamageEffect.h"
/*#include "GameplayTagContainer.h"    */      
/*#include "Engine/NetSerialization.h"     */  
/*#include "GameFramework/DamageType.h"  */    
#include "DamageSpec.generated.h"          
USTRUCT(BlueprintType)
struct FDamageSpec
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseDamage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) EDamageType DamageType;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> Instigator;

    UPROPERTY(BlueprintReadWrite)
    TWeakObjectPtr<AActor> Causer;

    UPROPERTY(BlueprintReadWrite)
    FName SourceSocket = NAME_None;
};