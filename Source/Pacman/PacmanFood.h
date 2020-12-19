#pragma once

#include "GameFramework/Actor.h"
#include "PacmanFood.generated.h"

class UStaticMeshComponent;

UCLASS()
class PACMAN_API APacmanFood : public AActor
{
	GENERATED_BODY()

public:
	APacmanFood();

	UPROPERTY(EditDefaultsOnly)
	uint32 Score = 1;

private:
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* MeshComponent = nullptr;
};
