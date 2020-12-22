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
	AGhostPawn();
	void SetInitialState();

private:
	friend class AGhostsManager;

	FVector CurrentDirection;
	FVector InitialLocation;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	EGhostColor GhostColor;

	UPROPERTY(EditAnywhere, Category = "Ghost")
	float Speed;

	UPROPERTY()
	USphereComponent* CollisionComponent;

	UPROPERTY()
	UPawnMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	virtual void BeginPlay() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};