#pragma once

#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;
class UPacmanHUDWidget;
class USphereComponent;
class APacmanFood;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
public:
	FVector CurrentDirection;
	uint32 Score;

	APacmanPawn();
	uint32 Kill();
	void Move(float DeltaTime);

private:
	FVector WantedDirection;
	FVector InitialLocation;
	uint32 NumLives;
	uint32 NumFoodLeft;

	UPROPERTY() UPawnMovementComponent* MovementComponent;
	UPROPERTY() USphereComponent* CollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	TSubclassOf<APacmanFood> SuperFoodClass;

	TSubclassOf<UUserWidget> HUDWidgetClass;
	UPROPERTY() UPacmanHUDWidget* HUDWidget;

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
