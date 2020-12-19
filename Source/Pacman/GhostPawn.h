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

private:
	FVector CurrentDirection;
	FVector WantedDirection;
	uint32 DirectionCounter;

	UPROPERTY()
	USphereComponent* CollisionComponent;

	UPROPERTY()
	UPawnMovementComponent* MovementComponent;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
};
