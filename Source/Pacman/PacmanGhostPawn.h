#pragma once
#include "GameFramework/Pawn.h"
#include "PacmanGhostPawn.generated.h"

class USphereComponent;
class UPawnMovementComponent;

UENUM()
enum class EGhostColor : uint8
{
	Red,
	Pink,
	Blue,
	Orange,
};

UCLASS()
class PACMAN_API AGhostPawn : public APawn
{
public:
	FVector CurrentDirection;
	float FrozenModeTimer;
	bool bIsFrightened;
	bool bIsInHouse;
	FVector HouseLocation;

	UPROPERTY(EditAnywhere, Category = "Ghost") EGhostColor Color;
	UPROPERTY(EditAnywhere, Category = "Ghost") FVector ScatterTargetLocation;
	UPROPERTY(EditAnywhere, Category = "Ghost") FVector SpawnLocation;

	UPROPERTY() UMaterialInstanceDynamic* Material;
	UPROPERTY() UMaterialInstanceDynamic* FrightenedMaterial;

	AGhostPawn();
	void MoveToGhostHouse();
	void Move(float DeltaTime);
	void SetDefaultMaterial();
	void SetFrightenedMaterial();

private:
	UPROPERTY() USphereComponent* CollisionComponent;
	UPROPERTY() UPawnMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, Category = "Ghost") float Speed;
	UPROPERTY(EditAnywhere, Category = "Ghost") float LeaveHouseTime;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	virtual void BeginPlay() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};

FORCEINLINE void AGhostPawn::SetDefaultMaterial()
{
	if (VisualComponent && Material)
	{
		VisualComponent->SetMaterial(0, Material);
	}
}

FORCEINLINE void AGhostPawn::SetFrightenedMaterial()
{
	if (VisualComponent && FrightenedMaterial)
	{
		VisualComponent->SetMaterial(0, FrightenedMaterial);
	}
}
