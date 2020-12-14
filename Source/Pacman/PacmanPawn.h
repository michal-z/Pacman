#pragma once

#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;
class ACameraActor;
class UPacmanHUDWidget;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
	FVector InputVector;
	FVector WantedInputVector;
	FVector NonBlockingInputVector;

	UPROPERTY(EditAnywhere)
	ACameraActor* MainCamera = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY()
	UPacmanHUDWidget* HUDWidget = nullptr;

	uint32 Score = 0;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

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
