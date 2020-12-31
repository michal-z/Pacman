#include "GhostPawn.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"
#include "PacmanPawn.h"

PRAGMA_DISABLE_OPTIMIZATION

AGhostPawn::AGhostPawn()
{
	This.PrimaryActorTick.bCanEverTick = false;

	This.CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	This.CollisionComponent->InitSphereRadius(49.0f);
	This.CollisionComponent->SetCollisionObjectType(ECC_WorldDynamic);
	This.CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	This.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	This.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	This.CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	This.CollisionComponent->SetAllUseCCD(true);

	This.VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	This.VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	This.VisualComponent->SetupAttachment(This.CollisionComponent);

	This.RootComponent = This.CollisionComponent;

	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		This.VisualComponent->SetStaticMesh(Finder.Object);
	}

	This.MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	This.MovementComponent->UpdatedComponent = This.RootComponent;

	This.AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	This.CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	This.Speed = 400.0f;
	This.bIsInHouse = true;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();

	This.HouseLocation = GetActorLocation();
	This.DefaultMaterial = This.VisualComponent->GetMaterial(0);
	This.FrozenModeTimer = This.LeaveHouseTime;
}

UPawnMovementComponent* AGhostPawn::GetMovementComponent() const
{
	return This.MovementComponent;
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

	SetActorLocation(This.HouseLocation, false, nullptr, ETeleportType::ResetPhysics);
	This.VisualComponent->SetMaterial(0, This.DefaultMaterial);
	This.CurrentDirection = CDO->CurrentDirection;
	This.bIsInHouse = true;
	This.bIsFrightened = false;
	This.FrozenModeTimer = This.LeaveHouseTime;
}
