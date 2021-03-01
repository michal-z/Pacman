#include "PacmanGhostPawn.h"
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
	CollisionComponent->InitSphereRadius(GMapTileSize / 2 - 1.0f);
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

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	Speed = 400.0f;
	bIsInHouse = true;
}

void AGhostPawn::BeginPlay()
{
	Super::BeginPlay();

	HouseLocation = GetActorLocation();

	Material = UMaterialInstanceDynamic::Create(VisualComponent->GetMaterial(0), this);
	VisualComponent->SetMaterial(0, Material);

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		FrightenedMaterial = UMaterialInstanceDynamic::Create(GameMode->GhostFrightenedMaterial, this);
	}

	FrozenModeTimer = LeaveHouseTime;
}

UPawnMovementComponent* AGhostPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void AGhostPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (bIsInHouse == false)
	{
		APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GameMode && OtherActor)
		{
			GameMode->HandleActorOverlap(OtherActor, this);
		}
	}
}

void AGhostPawn::MoveToGhostHouse()
{
	AGhostPawn* CDO = StaticClass()->GetDefaultObject<AGhostPawn>();

	SetActorLocation(HouseLocation, false, nullptr, ETeleportType::ResetPhysics);
	CurrentDirection = CDO->CurrentDirection;
	bIsInHouse = true;
	bIsFrightened = false;
	FrozenModeTimer = LeaveHouseTime;
}

void AGhostPawn::Move(float DeltaTime)
{
	const float GhostSpeed = Speed * (bIsFrightened ? 0.5f : 1.0f);
	const FVector Delta = CurrentDirection * GhostSpeed * DeltaTime;

	FHitResult Hit;
	MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
	}
}
