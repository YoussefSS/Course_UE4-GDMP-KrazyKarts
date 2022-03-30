// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "Components\InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Net\UnrealNetwork.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // Pawn replicates by default, but we do this anyway
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		NetUpdateFrequency = 1; // 1 update per second
	}
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoKart, ServerState); // This tells unreal that this variable should be replicated. Meaning when the client changes this value and replicates it, all the clients will get the updated value
	DOREPLIFETIME(AGoKart, Throttle);
	DOREPLIFETIME(AGoKart, SteeringThrow);
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "ERROR";
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled()) // We use this instead of checking for AutonomousProxy as it won't work for the server
	{
		FGoKartMove Move;
		Move.DeltaTime = DeltaTime;
		Move.SteeringThrow = SteeringThrow;
		Move.Throttle = Throttle;
		// TODO: Set Move.Time

		Server_SendMove(Move);
	}
	

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;

	Force += GetAirResistance();
	Force += GetRollingResistance();

	FVector Acceleration = Force / Mass;

	

	Velocity = Velocity + Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);

	if (HasAuthority())
	{
		// Pseudo Step: Sending the canonical state to the clients
		ServerState.Transform = GetActorTransform();
		ServerState.Velocity = Velocity;
		// TODO: Update lat move
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);

}

void AGoKart::OnRep_ServerState()
{
	// Pseudo Step: Reset to server state
	SetActorTransform(ServerState.Transform);
	Velocity = ServerState.Velocity;
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoefficient; // SizeSquared = Square(Velocity.Size())
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100;
	float NormalForce = Mass * AccelerationDueToGravity;
	return -Velocity.GetSafeNormal() * RollingResistanceCoefficient * NormalForce;
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	float DeltaLocation = FVector::DotProduct(GetActorForwardVector(), Velocity) * DeltaTime; // Dot gives what portion of the velocity vector is in the forward, if negative, will give negative number
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), RotationAngle); // RotationAngle is in degrees, while this function takes radians

	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * DeltaTime * 100; // Multiplying by deltatime makes it just meters now, multiplying by 100 makes it centimeters. So now we are adding this movement per tick (20 meters per second)

	FHitResult Hit;
	AddActorWorldOffset(Translation, true, &Hit);

	
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::ZeroVector;
	}
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	/** To update the AutonomousProxy, we should
		- Handle the bindings locally first
		- Then pass them up to the server 
		- We don't actually have to check if we are the AutonomousProxy here, 
			since we know the AutonomousProxy is the only one that has the controller attached, so it is the only one that can receive input from the player controller. */

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);

}



void AGoKart::MoveForward(float Value)
{
	Throttle = Value; // This means the player could cheat and set a high throttle value for themselves, but on the server we still have the validation in place
}

void AGoKart::MoveRight(float Value)
{
	SteeringThrow = Value;
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	Throttle = Move.Throttle;
	SteeringThrow = Move.SteeringThrow;
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO: Make better validation
}