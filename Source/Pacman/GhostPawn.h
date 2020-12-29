#pragma once

#include "GameFramework/Pawn.h"
#include "GhostPawn.generated.h"

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
	public: FVector CurrentDirection;
	public: float FrozenModeTimer;
	public: bool bIsInHouse;
	public: bool bIsFrightened;
	public: UMaterialInterface* DefaultMaterial;
	private: FVector HouseLocation;

	public: UPROPERTY(EditAnywhere, Category = "Ghost") float Speed;
	public: UPROPERTY(EditAnywhere, Category = "Ghost") float LeaveHouseTime;
	public: UPROPERTY(EditAnywhere, Category = "Ghost") EGhostColor Color;
	public: UPROPERTY(EditAnywhere, Category = "Ghost") FVector ScatterTargetLocation;
	public: UPROPERTY(EditAnywhere, Category = "Ghost") FVector SpawnLocation;

	public: UPROPERTY() USphereComponent* CollisionComponent;
	public: UPROPERTY() UPawnMovementComponent* MovementComponent;

	public: UPROPERTY(VisibleAnywhere, Category = "Components") UStaticMeshComponent* VisualComponent;


	public: AGhostPawn();
	public: void TeleportToHouse();
	private: virtual void BeginPlay() override;
	private: virtual UPawnMovementComponent* GetMovementComponent() const override;
	private: virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};
