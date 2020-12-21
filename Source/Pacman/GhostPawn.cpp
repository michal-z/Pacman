#include "GhostPawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"
#include "PacmanPawn.h"

PRAGMA_DISABLE_OPTIMIZATION

AGhostPawn::AGhostPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(49.0f);
	CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->SetAllUseCCD(true);

	UStaticMeshComponent* VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualComponent->SetupAttachment(CollisionComponent);

	RootComponent = CollisionComponent;


	static ConstructorHelpers::FObjectFinder<UStaticMesh> VisualAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (VisualAsset.Succeeded())
	{
		VisualComponent->SetStaticMesh(VisualAsset.Object);
	}

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	DirectionUpdateTime = 0.0f;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();

	InitialLocation = GetActorLocation();
}

void AGhostPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	APacmanPawn* Pacman = PC->GetPawn<APacmanPawn>();
	UWorld* World = GetWorld();

	if (PC == nullptr || Pacman == nullptr || World == nullptr)
	{
		return;
	}

	if ((DirectionUpdateTime += DeltaTime) > 0.25f)
	{
		DirectionUpdateTime = 0.0f;

		const float Radius = CollisionComponent->GetScaledSphereRadius();
		const FVector PacmanLocation = Pacman->GetActorLocation();
		const FVector GhostLocation = GetActorLocation();

		const FVector Destinations[3] =
		{
			GhostLocation + CurrentDirection * Radius * 0.5f,
			GhostLocation + FVector(CurrentDirection.Y, CurrentDirection.X, 0.0f) * Radius * 0.5f,
			GhostLocation + FVector(-CurrentDirection.Y, -CurrentDirection.X, 0.0f) * Radius * 0.5f,
		};
		float Distances[3] = {};
		bool bIsBlocked[3] = {};

		for (uint32 Idx = 0; Idx < 3; ++Idx)
		{
			bIsBlocked[Idx] = World->SweepTestByChannel(GhostLocation, Destinations[Idx], FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

			Distances[Idx] = FVector::Distance(Destinations[Idx], PacmanLocation);
		}

		int32 SelectedDirection = -1;
		float MinDistance = 100000.0f;
		for (uint32 Idx = 0; Idx < 3; ++Idx)
		{
			if (bIsBlocked[Idx] == false && Distances[Idx] < MinDistance)
			{
				MinDistance = Distances[Idx];
				SelectedDirection = (int32)Idx;
			}
		}
		check(SelectedDirection != -1);

		FVector WantedDirection;

		if (SelectedDirection == 0)
		{
			WantedDirection = CurrentDirection;
		}
		else if (SelectedDirection == 1)
		{
			WantedDirection = FVector(CurrentDirection.Y, CurrentDirection.X, 0.0f);
		}
		else if (SelectedDirection == 2)
		{
			WantedDirection = FVector(-CurrentDirection.Y, -CurrentDirection.X, 0.0f);
		}
		else
		{
			check(false);
		}

		CurrentDirection = WantedDirection;
	}

	const FVector Delta = CurrentDirection * DeltaTime * 400.0f;

	FHitResult Hit;
	MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
	}
}

UPawnMovementComponent* AGhostPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void AGhostPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		GameMode->KillPacman();
	}
}

void AGhostPawn::SetInitialState()
{
	AGhostPawn* CDO = StaticClass()->GetDefaultObject<AGhostPawn>();

	SetActorLocation(InitialLocation, false, nullptr, ETeleportType::ResetPhysics);
	CurrentDirection = CDO->CurrentDirection;
	DirectionUpdateTime = CDO->DirectionUpdateTime;
}
