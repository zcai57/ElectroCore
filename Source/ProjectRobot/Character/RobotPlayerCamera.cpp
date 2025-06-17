// Fill out your copyright notice in the Description page of Project Settings.


#include "RobotPlayerCamera.h"
#include "RobotPlayerMovement.h"
#include "RobotPlayerCharacter.h"
#include "Components/CapsuleComponent.h"

ARobotPlayerCamera::ARobotPlayerCamera()
{
}

void ARobotPlayerCamera::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
    Super::UpdateViewTarget(OutVT, DeltaTime);
    if (ARobotPlayerCharacter* RobotCharacter = Cast<ARobotPlayerCharacter>(GetOwningPlayerController()->GetPawn()))
    {
        URobotPlayerMovement* RMC = RobotCharacter->GetRobotPlayerMovement();
        FVector TargetCourchOffset = FVector(
            0,
            0,
            RMC->GetCrouchedHalfHeight() - RobotCharacter->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
    
        FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCourchOffset, FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.f, 1.f));
        if (RMC->IsCrouching())
        {
            CrouchBlendTime = FMath::Clamp(CrouchBlendTime + DeltaTime, 0.f, CrouchBlendDuration);
            Offset -= TargetCourchOffset;
        }
        else {
            CrouchBlendTime = FMath::Clamp(CrouchBlendTime - DeltaTime, 0.f, CrouchBlendDuration);
        }
        // Only changes this when on ground
        if (RMC->IsMovingOnGround())
        {
            OutVT.POV.Location += Offset;
        }
    }
}
