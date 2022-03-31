// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"

#include "Components\InputComponent.h"
#include "DrawDebugHelpers.h"
#include "Net\UnrealNetwork.h"
#include "GameFramework\GameStateBase.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // Pawn replicates by default, but we do this anyway

	MovementComponent = CreateDefaultSubobject<UGoKartMovementComponent>(TEXT("MovementComponent"));
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

	if (MovementComponent == nullptr) return;

	/* Since all pawns show up as Authority when you're the server, we need to know whether we are the controlling pawn, or just the authority server and the pawn is controlled by someone else
	 You could use GetRemoteRole() == ROLE_SimulatedProxy, which means you are not an AutonomousProxy on clients, which means you are the server, but this inconsistent, and it's better to use IsLocallyControlled */
	if (GetLocalRole() == ROLE_Authority && IsLocallyControlled()) // This means we are the server, and the ones in control of this pawn.
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move); // We don't have to call SimulateMove here as it is already called in Server_SendMove
	}


	// We are an AutonomousProxy, not a server
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComponent->CreateMove(DeltaTime);
		MovementComponent->SimulateMove(Move);

		UnacknowledgedMoves.Add(Move);
		Server_SendMove(Move);
	}


	// We need to simulate for the SimulatedProxy, because we weren't doing anything here, we were only setting the transform in OnRep_ServerState
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove); // WHY NOT CREATE A NEW MOVE?
	}

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(GetLocalRole()), this, FColor::White, DeltaTime);
}

void AGoKart::OnRep_ServerState()
{
	if (MovementComponent == nullptr) return;

	// Pseudo Step: Reset to server state
	SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgeMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move); // We cleared all the acknowledged moves, so we perform al the remaining unacknowledged ones
	}
}

void AGoKart::ClearAcknowledgeMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	// Adding only newer moves
	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgedMoves = NewMoves;
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
	if (MovementComponent == nullptr) return;

	MovementComponent->SetThrottle(Value); // This means the player could cheat and set a high throttle value for themselves, but on the server we still have the validation in place
}

void AGoKart::MoveRight(float Value)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SetSteeringThrow(Value);
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move);

	// After we simulated the move, we can update our canonical state
	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO: Make better validation
}