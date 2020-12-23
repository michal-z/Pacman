#pragma once

#include "GameFramework/Pawn.h"
#include "GhostPawn.generated.h"

class USphereComponent;
class UPawnMovementComponent;

UENUM()
enum class EGhostColor : uint8
{
	Red,
	Pink,
	Blue,
	Orange,
};

UCLASS()
class PACMAN_API AGhostPawn : public APawn
{
public:
	FVector CurrentDirection;
	float Frozen;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	float Speed;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	EGhostColor GhostColor;

	UPROPERTY() USphereComponent* CollisionComponent;
	UPROPERTY() UPawnMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	AGhostPawn();
	void SetInitialState();

private:
	FVector InitialLocation;

	virtual void BeginPlay() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};
