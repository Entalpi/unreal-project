// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShipPawn.generated.h"

UCLASS()
class MINIGOLD_API AShipPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AShipPawn();

	UPROPERTY(Category = Component, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* FloatingPawnMovement = nullptr;

	UPROPERTY(Category = Component, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* StaticMesh = nullptr;

	UPROPERTY(Category = Camera, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera = nullptr;
	
	UPROPERTY(Category = Camera, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraArm = nullptr;

	UPROPERTY(Category = General, VisibleAnywhere, meta = (AllowPrivateAccess = "true"))
	float Speed = 0.25f;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:		
	/* Fire a shot in the specified direction */
	void FireShot();

	void MoveForward(float Amount); 
	void Turn(float Amount);

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
