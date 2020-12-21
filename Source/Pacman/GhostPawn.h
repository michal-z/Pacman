#pragma once

#include "GameFramework/Pawn.h"
#include "GhostPawn.generated.h"

class USphereComponent;
class UPawnMovementComponent;

UCLASS()
class PACMAN_API AGhostPawn : public APawn
{
	GENERATED_BODY()

public:
	AGhostPawn();

	void SetInitialState();

private:
	FVector CurrentDirection;
	float DirectionUpdateTime;
	FVector InitialLocation;

	UPROPERTY()
	USphereComponent* CollisionComponent;

	UPROPERTY()
	UPawnMovementComponent* MovementComponent;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
