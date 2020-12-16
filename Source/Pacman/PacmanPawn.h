#pragma once

#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;
class UPacmanHUDWidget;
class USphereComponent;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
	FVector CurrentDirection;
	FVector WantedDirection;

	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UPacmanHUDWidget* HUDWidget = nullptr;

	uint32 Score = 0;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;


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

public:
	APacmanPawn();

	GENERATED_BODY()
};
