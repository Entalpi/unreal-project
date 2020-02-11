#include "MinigoldGameMode.h"
#include "MinigoldPawn.h"
#include "ShipPawn.h"

AMinigoldGameMode::AMinigoldGameMode()
{
	// Set default pawn class to our character class
	DefaultPawnClass = AMinigoldPawn::StaticClass();
	// DefaultPawnClass = AShipPawn::StaticClass();
}

