// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MinigoldGameMode.h"
#include "MinigoldPawn.h"

AMinigoldGameMode::AMinigoldGameMode()
{
	// set default pawn class to our character class
	DefaultPawnClass = AMinigoldPawn::StaticClass();
}

