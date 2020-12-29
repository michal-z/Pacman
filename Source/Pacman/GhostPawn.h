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
	float FrozenModeTimer;
	bool bIsInHouse;
	bool bIsFrightened;
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditAnywhere, Category = "Ghost") float Speed;
	UPROPERTY(EditAnywhere, Category = "Ghost") float LeaveHouseTime;
	UPROPERTY(EditAnywhere, Category = "Ghost") EGhostColor Color;
	UPROPERTY(EditAnywhere, Category = "Ghost") FVector ScatterTargetLocation;
	UPROPERTY(EditAnywhere, Category = "Ghost") FVector SpawnLocation;

	UPROPERTY() USphereComponent* CollisionComponent;
	UPROPERTY() UPawnMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	AGhostPawn();
	void TeleportToHouse();

private:
	FVector HouseLocation;

	virtual void BeginPlay() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};
