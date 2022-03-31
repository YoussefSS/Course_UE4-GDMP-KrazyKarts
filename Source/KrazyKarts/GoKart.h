// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKartMovementComponent.h"

#include "GoKart.generated.h"




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

	void ClearAcknowledgeMoves(FGoKartMove LastMove);

	void MoveForward(float Value);

	void MoveRight(float Value);

	/** Reliable server RPC function */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FGoKartMove Move);

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FGoKartState ServerState; // We replicate this as it holds all the information

	/*
	UPROPERTY(ReplicatedUsing=OnRep_ReplicatedTransform)
	FTransform ReplicatedTransform;
	*/
	UFUNCTION()
	void OnRep_ServerState();

	TArray<FGoKartMove> UnacknowledgedMoves; // only on the client
	
	UPROPERTY(VisibleAnywhere)
	UGoKartMovementComponent* MovementComponent;

};
