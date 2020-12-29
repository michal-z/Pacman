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
	public: FVector CurrentDirection;
	public: uint32 Score;
	private: FVector WantedDirection;
	private: FVector InitialLocation;
	private: uint32 NumLives;
	private: uint32 NumFoodLeft;
	private: TSubclassOf<APacmanFood> SuperFoodClass;
	private: TSubclassOf<UUserWidget> HUDWidgetClass;

	private: UPROPERTY() UPawnMovementComponent* MovementComponent;
	private: UPROPERTY() USphereComponent* CollisionComponent;
	private: UPROPERTY() UPacmanHUDWidget* HUDWidget;

	private: UPROPERTY(VisibleAnywhere, Category = "Components") UStaticMeshComponent* VisualComponent;


	public: APacmanPawn();
	public: uint32 Kill();
	public: void Move(float DeltaTime);
	private: void MoveUp();
	private: void MoveDown();
	private: void MoveRight();
	private: void MoveLeft();
	private: virtual void BeginPlay() override;
	private: virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	private: virtual UPawnMovementComponent* GetMovementComponent() const override;
	private: virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};
