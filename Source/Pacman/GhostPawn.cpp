#include "GhostPawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"
#include "PacmanPawn.h"

PRAGMA_DISABLE_OPTIMIZATION

AGhostPawn::AGhostPawn()
{
	Self.PrimaryActorTick.bCanEverTick = false;

	Self.CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	Self.CollisionComponent->InitSphereRadius(49.0f);
	Self.CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	Self.CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	Self.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Self.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Self.CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Self.CollisionComponent->SetAllUseCCD(true);

	Self.VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	Self.VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Self.VisualComponent->SetupAttachment(CollisionComponent);

	Self.RootComponent = CollisionComponent;

	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		Self.VisualComponent->SetStaticMesh(Finder.Object);
	}

	Self.MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	Self.MovementComponent->UpdatedComponent = RootComponent;

	Self.AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	Self.CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	Self.Speed = 400.0f;
	Self.bIsInHouse = true;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();

	Self.HouseLocation = GetActorLocation();
	Self.DefaultMaterial = VisualComponent->GetMaterial(0);
	Self.FrozenModeTimer = LeaveHouseTime;
}

UPawnMovementComponent* AGhostPawn::GetMovementComponent() const
{
	return Self.MovementComponent;
}

void AGhostPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		GameMode->NotifyGhostBeginOverlap(OtherActor, this);
	}
}

void AGhostPawn::TeleportToHouse()
{
	AGhostPawn* CDO = StaticClass()->GetDefaultObject<AGhostPawn>();

	SetActorLocation(Self.HouseLocation, false, nullptr, ETeleportType::ResetPhysics);
	Self.VisualComponent->SetMaterial(0, Self.DefaultMaterial);
	Self.CurrentDirection = CDO->CurrentDirection;
	Self.bIsInHouse = true;
	Self.bIsFrightened = false;
	Self.FrozenModeTimer = Self.LeaveHouseTime;
}
