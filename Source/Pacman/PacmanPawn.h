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
private:
	FVector WantedDirection;
	FVector InitialLocation;
	uint32 NumLives;
	uint32 NumFoodLeft;
	TSubclassOf<APacmanFood> SuperFoodClass;
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY() UPawnMovementComponent* MovementComponent;
	UPROPERTY() USphereComponent* CollisionComponent;
	UPROPERTY() UPacmanHUDWidget* HUDWidget;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

public:
	APacmanPawn();
	uint32 Kill();
	void Move(float DeltaTime);
private:
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
