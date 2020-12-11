#pragma once

#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
	FVector InputVector;
	FVector WantedInputVector;
	FVector NonBlockingInputVector;

public:
	APacmanPawn();

	UPROPERTY()
	UPawnMovementComponent* MovementComponent;

	void MoveUp();
	void MoveDown();
	void MoveRight();
	void MoveLeft();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	GENERATED_BODY()
};
