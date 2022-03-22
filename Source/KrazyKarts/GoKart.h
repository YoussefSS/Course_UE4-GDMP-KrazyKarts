// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	

	

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


private:
	FVector GetAirResistance();

	FVector GetRollingResistance();

	void ApplyRotation(float DeltaTime);

	void UpdateLocationFromVelocity(float DeltaTime);

	// The mass of the car (kg)
	UPROPERTY(EditAnywhere)
	float Mass = 1000;

	// The force applied tot he car when the throttle is fully down (N).
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000; //10000/1000 gives an acceleration of 10
	
	// The numbers of degrees rotated per second at full control throw (degrees/s)
	UPROPERTY(EditAnywhere)
	float MaxDegreesPerSecond = 90;

	// Higher means more drag. (kg/m)
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16; // = 10000 / 25^2

	// Higher means more rolling resistance
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015; // From wikipedia rolling resistance (0.01 to 0.015

	FVector Velocity;

	void MoveForward(float Value);

	void MoveRight(float Value);

	float Throttle;
	float SteeringThrow;
};
