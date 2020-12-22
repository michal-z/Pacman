#pragma once

#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;
class UPacmanHUDWidget;
class USphereComponent;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
public:
	APacmanPawn();
	uint32 Kill();

private:
	friend class AGhostsManager;

	FVector CurrentDirection;
	FVector WantedDirection;
	FVector InitialLocation;

	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UPacmanHUDWidget* HUDWidget;

	uint32 Score;
	uint32 NumLives;

	UPROPERTY()
	UPawnMovementComponent* MovementComponent;

	UPROPERTY()
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	void MoveUp();
	void MoveDown();
	void MoveRight();
	void MoveLeft();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};
