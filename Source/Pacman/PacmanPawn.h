#pragma once
#include "GameFramework/Pawn.h"
#include "PacmanPawn.generated.h"

class UPawnMovementComponent;
class USphereComponent;

UCLASS()
class PACMAN_API APacmanPawn : public APawn
{
public:
	APacmanPawn();
	void Move(float DeltaTime);
	void MoveToStartLocation();
	void SetDefaultMaterial();
	void SetTeleportMaterial();
	FVector GetCurrentDirection() const;
	UMaterialInstanceDynamic* GetTeleportMaterial() const;

private:
	UMaterialInterface* DefaultMaterial;
	UPROPERTY() UMaterialInstanceDynamic* TeleportMaterial;
	FVector InitialLocation;
	FVector CurrentDirection;
	FVector WantedDirection;
	UPROPERTY() UPawnMovementComponent* MovementComponent;
	UPROPERTY() USphereComponent* CollisionComponent;

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

FORCEINLINE FVector APacmanPawn::GetCurrentDirection() const
{
	return CurrentDirection;
}

FORCEINLINE UMaterialInstanceDynamic* APacmanPawn::GetTeleportMaterial() const
{
	return TeleportMaterial;
}
