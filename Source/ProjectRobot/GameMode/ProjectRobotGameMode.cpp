// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectRobotGameMode.h"
//#include "../ProjectRobotCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProjectRobotGameMode::AProjectRobotGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/AProjectRobotGame/Character/BP_RobotPlayer"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
