#include "PacmanFood.h"

APacmanFood::APacmanFood()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);

	RootComponent = MeshComponent;

	Score = 1;
}
