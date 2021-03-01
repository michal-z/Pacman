#pragma once
#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;
class USphereComponent;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
public:
	FVector CurrentDirection;

	UPROPERTY()
	UMaterialInstanceDynamic* Material;

	APacmanPawn();
	void Move(float DeltaTime);
	void MoveToStartLocation();
	void SelectRandomDirection(const FVector& Location);

private:
	UPROPERTY() UPawnMovementComponent* MovementComponent;
	UPROPERTY() USphereComponent* CollisionComponent;

	FVector InitialLocation;
	FVector WantedDirection;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* VisualComponent;

	void MoveUp();
	void MoveDown();
	void MoveRight();
	void MoveLeft();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};
