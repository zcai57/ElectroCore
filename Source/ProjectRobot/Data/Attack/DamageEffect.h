#pragma once
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	EDT_None UMETA(DisplayName = "None"),
	EDT_Slow UMETA(DisplayName = "Slow"),
	EDT_Knockback UMETA(DisplayName = "Knockback"),
	EDT_Stunned UMETA(DisplayName = "Stunned"),
};