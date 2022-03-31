// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoKartMovementComponent.generated.h"


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


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KRAZYKARTS_API UGoKartMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGoKartMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SimulateMove(const FGoKartMove& Move);

	FVector GetVelocity() { return Velocity; }
	void SetVelocity(FVector Val) { Velocity = Val; }

	void SetThrottle(float Val) { Throttle = Val; }
	void SetSteeringThrow(float Val) { SteeringThrow = Val; }

	FGoKartMove GetLastMove() { return LastMove; }

private:

	FGoKartMove CreateMove(float DeltaTime);

	FVector GetAirResistance();

	FVector GetRollingResistance();

	void ApplyRotation(float DeltaTime, float InSteeringThrow);

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
	float RollingResistanceCoefficient = 0.015; // From wikipedia rolling resistance (0.01 to 0.015)

	FVector Velocity; // We keep this, but we keep it in sync with the server. So we replace that when we get the replicated state

	float Throttle;

	float SteeringThrow;

	FGoKartMove LastMove;
	
};
