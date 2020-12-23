#pragma once

#include "GameFramework/Actor.h"
#include "PacmanFood.generated.h"

class UStaticMeshComponent;

UCLASS()
class PACMAN_API APacmanFood : public AActor
{
public:
	UPROPERTY(EditDefaultsOnly, Category = "Pacman Food")
	uint32 Score;

	APacmanFood();

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	GENERATED_BODY()
};
