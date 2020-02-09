// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MinigoldPawn.h"
#include "MinigoldProjectile.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

const FName AMinigoldPawn::MoveForwardBinding("MoveForward");
const FName AMinigoldPawn::MoveTurnBinding("MoveTurn");
const FName AMinigoldPawn::FireBinding("FireCannons");

AMinigoldPawn::AMinigoldPawn()
{	
	// Create the mesh component
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/Goldship/Models/ship_light_ship_light_8angles.ship_light_ship_light_8angles"));
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	RootComponent = ShipMeshComponent;
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);

	// Create sails
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipSailsMesh(TEXT("/Game/Goldship/Models/ship_light_sails"));
    const auto SailsMesh =  CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipSailsMesh"));
	
	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Goldship/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(false); // Want arm to rotate when ship does
	CameraBoom->TargetArmLength = 1400.f;
	CameraBoom->SetRelativeRotation(FRotator(-80.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;	// Camera does not rotate relative to arm

	// Movement
	MoveSpeed = 1000.0f;
	// Weapon
	GunOffset = FVector(90.f, 0.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;
}

void AMinigoldPawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	// set up gameplay key bindings
	PlayerInputComponent->BindAxis(MoveForwardBinding);
	PlayerInputComponent->BindAxis(MoveTurnBinding);
	PlayerInputComponent->BindAction(FireBinding, IE_Pressed, this, &AMinigoldPawn::FireShot);
}

void AMinigoldPawn::Tick(float DeltaSeconds)
{
	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float TurningValue = GetInputAxisValue(MoveTurnBinding);

	// Rotate 
	const float Pitch = 0.0f;
	const float Yaw = TurningValue;
	const float Roll = 0.0;
	const FRotator DeltaRotation = FRotator(Pitch, Yaw, Roll);
	RootComponent->AddWorldRotation(DeltaRotation);

	// Calculate movement
	const FVector Movement = -GetActorRightVector() * ForwardValue * MoveSpeed * DeltaSeconds;
	FHitResult Hit(1.f);
	const bool sweep = true;
	RootComponent->AddWorldOffset(Movement, sweep, &Hit); // Sweep = collision detection
	
	if (Hit.IsValidBlockingHit())
	{
		const FVector Normal2D = Hit.Normal.GetSafeNormal2D();
		const FVector Deflection = FVector::VectorPlaneProject(Movement, Normal2D) * (1.f - Hit.Time);
		RootComponent->AddRelativeLocation(Deflection);
	}
}

void AMinigoldPawn::FireShot()
{
	UE_LOG(LogTemp, Warning, TEXT("Yeah boi!"));

	// If it's ok to fire again
	if (bCanFire == true)
	{
		const FRotator FireRotation = FRotator(0.0f, 0.0f, 0.0f);
		// Spawn projectile at an offset from this pawn
		const FVector SpawnLocation = GetActorLocation() + FireRotation.RotateVector(GunOffset);
		// UE_LOG(LogTemp, Warning, "%s", GetActorLocation());

		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			// spawn the projectile
			World->SpawnActor<AMinigoldProjectile>(SpawnLocation, FireRotation);
		}

		bCanFire = false;
		World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AMinigoldPawn::ShotTimerExpired, FireRate);

		// try and play the sound if specified
		if (FireSound != nullptr)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		bCanFire = false;
	}
}

void AMinigoldPawn::ShotTimerExpired()
{
	bCanFire = true;
}

