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
	APacmanPawn();
	uint32 Kill();
	void Move(float DeltaTime);
	void AddScore(uint32 InScore);
	void SetDefaultMaterial();
	void SetTeleportMaterial();
	FVector GetInitialLocation() const;
	FVector GetCurrentDirection() const;
	UMaterialInstanceDynamic* GetTeleportMaterial() const;

private:
	UMaterialInterface* DefaultMaterial;
	UPROPERTY() UMaterialInstanceDynamic* TeleportMaterial;
	FVector InitialLocation;
	FVector CurrentDirection;
	FVector WantedDirection;
	uint32 Score;
	uint32 NumLives;
	uint32 NumFoodLeft;
	TSubclassOf<APacmanFood> SuperFoodClass;
	TSubclassOf<UUserWidget> HUDWidgetClass;
	UPROPERTY() UPawnMovementComponent* MovementComponent;
	UPROPERTY() USphereComponent* CollisionComponent;
	UPROPERTY() UPacmanHUDWidget* HUDWidget;

	UPROPERTY(VisibleAnywhere, Category = "Components")
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

FORCEINLINE void APacmanPawn::SetDefaultMaterial()
{
	VisualComponent->SetMaterial(0, DefaultMaterial);
}

FORCEINLINE void APacmanPawn::SetTeleportMaterial()
{
	VisualComponent->SetMaterial(0, TeleportMaterial);
}

FORCEINLINE FVector APacmanPawn::GetInitialLocation() const
{
	return InitialLocation;
}

FORCEINLINE FVector APacmanPawn::GetCurrentDirection() const
{
	return CurrentDirection;
}

FORCEINLINE UMaterialInstanceDynamic* APacmanPawn::GetTeleportMaterial() const
{
	return TeleportMaterial;
}
