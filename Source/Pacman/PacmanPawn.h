#pragma once

#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;
class UPacmanHUDWidget;
class USphereComponent;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
	GENERATED_BODY()

public:
	APacmanPawn();

private:
	FVector CurrentDirection;
	FVector WantedDirection;

	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UPacmanHUDWidget* HUDWidget = nullptr;

	uint32 Score = 0;

	UPROPERTY()
	UPawnMovementComponent* MovementComponent;

	UPROPERTY()
	USphereComponent* CollisionComponent;

	void MoveUp();
	void MoveDown();
	void MoveRight();
	void MoveLeft();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
};
