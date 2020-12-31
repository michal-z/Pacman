#include "PacmanFood.h"

APacmanFood::APacmanFood()
{
	Self.PrimaryActorTick.bCanEverTick = false;

	Self.MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	Self.MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Self.MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	Self.MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	Self.MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Self.RootComponent = MeshComponent;

	Self.Score = 1;
}
