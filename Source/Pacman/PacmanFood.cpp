#include "PacmanFood.h"

APacmanFood::APacmanFood()
{
	This.PrimaryActorTick.bCanEverTick = false;

	This.MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	This.MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	This.MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	This.MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	This.MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	This.RootComponent = This.MeshComponent;

	This.Score = 1;
}
