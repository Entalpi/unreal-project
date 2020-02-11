#include "ShipPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/FloatingPawnMovement.h"

AShipPawn::AShipPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	FloatingPawnMovement = CreateDefaultSubobject<UFloatingPawnMovement>("FloatingPawnMovement");

	// Create the mesh component
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ShipMesh(TEXT("/Game/Goldship/Models/ship_light"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShipMesh"));
	StaticMesh->SetWorldScale3D(FVector(10.0f, 10.0f, 10.0f));
	StaticMesh->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	StaticMesh->SetStaticMesh(ShipMesh.Object);
	SetRootComponent(StaticMesh);

	CameraArm = CreateDefaultSubobject<USpringArmComponent>("CameraArm");
	CameraArm->TargetArmLength = 500.0f;
	CameraArm->SetupAttachment(RootComponent);

	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(CameraArm);
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AShipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FloatingPawnMovement->AddInputVector(GetActorForwardVector() * Speed);
}


void AShipPawn::FireShot()
{
	UE_LOG(LogTemp, Warning, TEXT("Yeah boi!"));
}

void AShipPawn::MoveForward(float Amount)
{
	FloatingPawnMovement->AddInputVector(GetActorForwardVector() * Speed);
}

void AShipPawn::Turn(float Amount)
{
	// TODO: ...
}

// Called to bind functionality to input
void AShipPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Forward", this, &AShipPawn::MoveForward);
	PlayerInputComponent->BindAxis("Turn", this, &AShipPawn::Turn);
	PlayerInputComponent->BindAction("FireCannons", IE_Pressed, this, &AShipPawn::FireShot);
}

