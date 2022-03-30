// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"


USTRUCT()
struct FGoKartMove
{
	GENERATED_BODY()

	/** We include the controls needed for simulation, we even include 
	DeltaTime as it may be different between the client and the server */

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float SteeringThrow;

	UPROPERTY()
	float DeltaTime; // To be able to simulate the move

	/** What if by chance, 2 moves were exactly the same ? We need to add something to identify them
	*We use a Time var, so that when we receive the last move from the server, we can go through our list of unacknowledged moves, 
		and check if they are before or equal to that last move. If they are before, we can remove them as they are old moves
		And the greater/newer ones are the ones which will stay in that list and get replayed */
	UPROPERTY()
	float Time;
};


USTRUCT()
struct FGoKartState
{
	GENERATED_BODY()

	/** What comes down from the server */

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	/** Include the Move in order for the NON AutonomousProxy to be able to simulate. 
	We are going to need the throttle in order to interpolate.
	This is the last move that went into making this state */
	UPROPERTY()
	FGoKartMove LastMove;
};

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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


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
	
	// Minimum radius of the car turning curcle at full lock (m)
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10; // 10 meters

	// Higher means more drag. (kg/m)
	UPROPERTY(EditAnywhere)
	float DragCoefficient = 16; // = 10000 / 25^2

	// Higher means more rolling resistance
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoefficient = 0.015; // From wikipedia rolling resistance (0.01 to 0.015

	void MoveForward(float Value);

	void MoveRight(float Value);

	/** Reliable server RPC function */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState; // We replicate this as it holds all the information

	
	FVector Velocity; // We keep this, but we keep it in sync with the server. So we replace that when we get the replicated state
	/*
	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedTransform)
	FTransform ReplicatedTransform;
	*/
	UFUNCTION()
	void OnRep_ServerState();
	

	UPROPERTY(Replicated)
	float Throttle;

	UPROPERTY(Replicated)
	float SteeringThrow;

	
};
