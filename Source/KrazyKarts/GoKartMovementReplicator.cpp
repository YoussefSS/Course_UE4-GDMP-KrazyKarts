// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKartMovementReplicator.h"
#include "Net\UnrealNetwork.h"

// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<UGoKartMovementComponent>();
	
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoKartMovementReplicator, ServerState); // This tells unreal that this variable should be replicated. Meaning when the client changes this value and replicates it, all the clients will get the updated value
}

// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


	if (MovementComponent == nullptr) return;

	FGoKartMove LastMove = MovementComponent->GetLastMove();

	// We are an AutonomousProxy, not a server
	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		/* Add move to the queue and send it to the server (!!!) where
		   it would be simulated as Server-side ("Canonical" simulation) code */

		UnacknowledgedMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	/* Since all pawns show up as Authority when you're the server, we need to know whether we are the controlling pawn, or just the authority server and the pawn is controlled by someone else
	 You could use GetRemoteRole() == ROLE_SimulatedProxy, which means you are not an AutonomousProxy on clients, which means you are the server, but this inconsistent, and it's better to use IsLocallyControlled */
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy) // This means we are the server, and the ones in control of this pawn.
	{
		UpdateServerState(LastMove);
	}

	// We need to simulate for the SimulatedProxy, because we weren't doing anything here, we were only setting the transform in OnRep_ServerState
	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove); // WHY NOT CREATE A NEW MOVE?
	}

}

void UGoKartMovementReplicator::UpdateServerState(const FGoKartMove& Move)
{
	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComponent->GetVelocity();
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (MovementComponent == nullptr) return;

	// Pseudo Step: Reset to server state
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetVelocity(ServerState.Velocity);

	ClearAcknowledgeMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComponent->SimulateMove(Move); // We cleared all the acknowledged moves, so we perform al the remaining unacknowledged ones
	}
}

void UGoKartMovementReplicator::ClearAcknowledgeMoves(FGoKartMove LastMove)
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

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move);

	UpdateServerState(Move);
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	return true; // TODO: Make better validation
}

