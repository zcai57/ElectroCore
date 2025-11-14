#pragma once
#include "CoreMinimal.h"
#include "TraceWindow.generated.h"          
USTRUCT(BlueprintType)
struct FTraceWindow {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName  Socket = "WeaponTip";
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float Range = 120.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector BoxHalfExtent = FVector(50.f, 50.f, 50.f);
	// Optional filters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;
};
