// Fill out your copyright notice in the Description page of Project Settings.


#include "GoKart.h"
#include "Components\InputComponent.h"

// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxdrivingForce * Throttle;

	FVector Acceleration = Force / Mass;

	Velocity = Velocity + Acceleration * DeltaTime;

	UpdateLocationFromVelocity(DeltaTime);


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

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);

}

void AGoKart::MoveForward(float Value)
{
	Throttle = Value;
	//Velocity = GetActorForwardVector() * 20 * Value; // Meters per second
}
