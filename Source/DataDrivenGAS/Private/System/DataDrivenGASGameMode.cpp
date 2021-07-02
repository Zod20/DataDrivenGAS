// Copyright Epic Games, Inc. All Rights Reserved.

#include "System/DataDrivenGASGameMode.h"
#include "Character/DataDrivenGASCharacter.h"
#include "UObject/ConstructorHelpers.h"

ADataDrivenGASGameMode::ADataDrivenGASGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

}
