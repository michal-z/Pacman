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
public:
	FVector CurrentDirection;
	float FrozenModeTimer;

	AGhostPawn();
	void SetInHouseState();
	void EnableFrightenedMode();
	void DisableFrightenedMode();
	void Move(float DeltaTime);
	float GetRadius() const;
	EGhostColor GetColor() const;
	FVector GetScatterTargetLocation() const;
	float GetSpeed() const;
	bool IsFrightened() const;

private:
	bool bIsInHouse;
	bool bIsFrightened;
	FVector HouseLocation;
	UMaterialInterface* DefaultMaterial;
	UMaterialInstance* FrightenedModeMaterial;
	UPROPERTY() UMaterialInstanceDynamic* TeleportMaterial;
	UPROPERTY() USphereComponent* CollisionComponent;
	UPROPERTY() UPawnMovementComponent* MovementComponent;

	UPROPERTY(EditAnywhere, Category = "Ghost") float Speed;
	UPROPERTY(EditAnywhere, Category = "Ghost") float LeaveHouseTime;
	UPROPERTY(EditAnywhere, Category = "Ghost") EGhostColor Color;
	UPROPERTY(EditAnywhere, Category = "Ghost") FVector ScatterTargetLocation;
	UPROPERTY(EditAnywhere, Category = "Ghost") FVector SpawnLocation;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* VisualComponent;

	virtual void BeginPlay() override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	GENERATED_BODY()
};

FORCEINLINE EGhostColor AGhostPawn::GetColor() const
{
	return Color;
}

FORCEINLINE FVector AGhostPawn::GetScatterTargetLocation() const
{
	return ScatterTargetLocation;
}

FORCEINLINE float AGhostPawn::GetSpeed() const
{
	return Speed;
}

FORCEINLINE void AGhostPawn::EnableFrightenedMode()
{
	if (bIsInHouse == false)
	{
		bIsFrightened = true;
		VisualComponent->SetMaterial(0, FrightenedModeMaterial);
	}
}

FORCEINLINE void AGhostPawn::DisableFrightenedMode()
{
	bIsFrightened = false;
	VisualComponent->SetMaterial(0, DefaultMaterial);
}

FORCEINLINE bool AGhostPawn::IsFrightened() const
{
	return bIsFrightened;
}
