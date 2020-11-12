#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class UPawnMovementComponent* MovementComponent;

private:
	FVector InputVector;
	FVector WantedInputVector;
	FVector NonBlockingInputVector;

public:
	APacmanPawn();

	void MoveUp();
	void MoveDown();
	void MoveRight();
	void MoveLeft();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
};
