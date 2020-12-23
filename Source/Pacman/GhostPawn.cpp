#include "GhostPawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"
#include "PacmanPawn.h"

PRAGMA_DISABLE_OPTIMIZATION

AGhostPawn::AGhostPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(49.0f);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetAllUseCCD(true);

	VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualComponent->SetupAttachment(CollisionComponent);

	RootComponent = CollisionComponent;

	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		check(Finder.Object);
		VisualComponent->SetStaticMesh(Finder.Object);
	}

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	Speed = 400.0f;
	Frozen = 0.0f;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
}

UPawnMovementComponent* AGhostPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void AGhostPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (Cast<APacmanPawn>(OtherActor))
	{
		APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GameMode)
		{
			GameMode->KillPacman();
		}
	}
	else
	{
		AGhostPawn* OtherGhost = Cast<AGhostPawn>(OtherActor);
		if (OtherGhost)
		{
			if (OtherGhost->Speed < Speed)
			{
				OtherGhost->Frozen = 0.5f;
			}
			else
			{
				Frozen = 0.5f;
			}
		}
	}
}

void AGhostPawn::SetInitialState()
{
	AGhostPawn* CDO = StaticClass()->GetDefaultObject<AGhostPawn>();

	SetActorLocation(InitialLocation, false, nullptr, ETeleportType::ResetPhysics);
	CurrentDirection = CDO->CurrentDirection;
	Frozen = CDO->Frozen;
}
