#pragma once

UENUM(BlueprintType)
enum class EActionState : uint8
{
    EAS_Unoccupied UMETA(DisplayName = "Unoccupied"),
    EAS_Attacking UMETA(DisplayName = "Attacking"),
};

UENUM(BlueprintType)
enum class ERobotState: uint8
{
    ERS_Idle UMETA(DisplayName = "Idle"),
    ERS_Drive UMETA(DisplayName = "Drive"),
    ERS_OverDrive UMETA(DisplayName = "OverDrive"),
};
