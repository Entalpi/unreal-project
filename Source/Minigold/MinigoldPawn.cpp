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
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <math.h>
#include <complex>

const FName AMinigoldPawn::MoveForwardBinding("MoveForward");
const FName AMinigoldPawn::MoveTurnBinding("MoveTurn");
const FName AMinigoldPawn::FireBinding("FireCannons");

FVector AMinigoldPawn::GetShipForwardVector() const
{
	FVector Forward = -GetActorRightVector();
	Forward.Z = 0.0f; // Movement is only allowed in X-Y-plane
	return Forward;
}

AMinigoldPawn::AMinigoldPawn()
{	
	// FIXME: Set Ship at water level

	// Cache our sound effect
	static ConstructorHelpers::FObjectFinder<USoundBase> FireAudio(TEXT("/Game/Goldship/Audio/TwinStickFire.TwinStickFire"));
	FireSound = FireAudio.Object;

	// Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/Goldship/Models/ship_light"));
	ShipMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	ShipMeshComponent->SetWorldScale3D(FVector(1.0f));
	ShipMeshComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	ShipMeshComponent->SetStaticMesh(ShipMesh.Object);
	ShipMeshComponent->SetSimulatePhysics(true);
	SetRootComponent(ShipMeshComponent);

	// Camera arm
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->TargetArmLength = 2000.f;
	CameraBoom->SetRelativeRotation(FRotator(-25.f, 270.f, 0.f)); 
	CameraBoom->bDoCollisionTest = false; // Want to pull camera in when it collides with level
	CameraBoom->SetupAttachment(RootComponent);

	// Camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Movement
	MoveSpeed = 1000.0f;
	TurnSpeed = 0.75f;
	// Weapon
	GunOffset = FVector(0.f, 90.f, 0.f);
	FireRate = 0.1f;
	bCanFire = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
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
	if (Health == 0)
	{
		Destroy();
	}

	// Simulate wave motion
	static float Time = 0.0f; 
	Time += DeltaSeconds;
	const float Amplitude = 0.025f;
	const float Pitching = Amplitude * std::sin(Time);
	const float Rolling = Amplitude * std::cos(Time);
	const FRotator Motions = FRotator(Pitching, 0.0f, Rolling);
	// RootComponent->AddLocalRotation(Motions);

	// Find movement direction
	const float ForwardValue = GetInputAxisValue(MoveForwardBinding);
	const float TurningValue = GetInputAxisValue(MoveTurnBinding);

	// Rotate 
	const float Pitch = 0.0f;
	const float Yaw = TurnSpeed * TurningValue;
	const float Roll = 0.0f;
	const FRotator DeltaRotation = FRotator(Pitch, Yaw, Roll);
	RootComponent->AddWorldRotation(DeltaRotation);

	// Calculate movement
	const FVector Movement = GetShipForwardVector() * ForwardValue * MoveSpeed * DeltaSeconds;
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
	if (bCanFire)
	{
		UWorld* const World = GetWorld();
		if (World)
		{ 
			// Forward cannon
			const FRotator FireRotation = GetShipForwardVector().Rotation();
			const FVector CannonOffset = FVector(0.0f, 0.0f, 174.0f) + GetShipForwardVector() * 550.0f;
			const FVector SpawnLocation = CannonOffset + GetActorLocation();
			const auto projectile = World->SpawnActor<AMinigoldProjectile>(SpawnLocation, FireRotation);

			// Cooldown
			bCanFire = false;
			World->GetTimerManager().SetTimer(TimerHandle_ShotTimerExpired, this, &AMinigoldPawn::ShotTimerExpired, FireRate);

			if (FireSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
			}
		}
	}
}

void AMinigoldPawn::ShotTimerExpired()
{
	bCanFire = true;
}

float AMinigoldPawn::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser)
{
	if (Health >= DamageAmount) 
	{
		Health -= static_cast<uint32_t>(DamageAmount);
	}
	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Red, FString::Printf(TEXT("Ship hp: %u"), Health));
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
