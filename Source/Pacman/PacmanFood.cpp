#include "PacmanFood.h"

APacmanFood::APacmanFood()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));

	RootComponent = MeshComponent;
}
