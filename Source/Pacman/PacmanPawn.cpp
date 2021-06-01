#include "PacmanPawn.h"
#include "PacmanGameModeBase.h"
#include "PacmanMiscClasses.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/FloatingPawnMovement.h"

#define LOCTEXT_NAMESPACE "PacmanPawn"

APacmanPawn::APacmanPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(GMapTileSize / 2 - 1.0f);
	CollisionComponent->SetCollisionObjectType(ECC_Pawn);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	CollisionComponent->SetAllUseCCD(true);

	VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualComponent->SetupAttachment(CollisionComponent);

	RootComponent = CollisionComponent;

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	WantedDirection = CurrentDirection;
}

void APacmanPawn::MoveUp()
{
	WantedDirection = FVector(0.0f, -1.0f, 0.0f);
}

void APacmanPawn::MoveDown()
{
	WantedDirection = FVector(0.0f, 1.0f, 0.0f);
}

void APacmanPawn::MoveRight()
{
	WantedDirection = FVector(1.0f, 0.0f, 0.0f);
}

void APacmanPawn::MoveLeft()
{
	WantedDirection = FVector(-1.0f, 0.0f, 0.0f);
}

void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()) != TEXT("Main"))
	{
		InitialLocation = GetActorLocation();
		Material = UMaterialInstanceDynamic::Create(VisualComponent->GetMaterial(0), this);
		VisualComponent->SetMaterial(0, Material);
	}
}

void APacmanPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("MoveUp"), IE_Pressed, this, &APacmanPawn::MoveUp);
	PlayerInputComponent->BindAction(TEXT("MoveDown"), IE_Pressed, this, &APacmanPawn::MoveDown);
	PlayerInputComponent->BindAction(TEXT("MoveRight"), IE_Pressed, this, &APacmanPawn::MoveRight);
	PlayerInputComponent->BindAction(TEXT("MoveLeft"), IE_Pressed, this, &APacmanPawn::MoveLeft);

	APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	PlayerInputComponent->BindAction(TEXT("PauseGame"), IE_Pressed, GameMode, &APacmanGameModeBase::PauseGame);
	PlayerInputComponent->BindAction(TEXT("RandomTeleport"), IE_Pressed, GameMode, &APacmanGameModeBase::DoRandomTeleport);
}

UPawnMovementComponent* APacmanPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void APacmanPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APacmanFood* Food = Cast<APacmanFood>(OtherActor);
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode && Food)
	{
		GameMode->HandleActorOverlap(this, Food);
	}
}

void APacmanPawn::Move(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const float Radius = CollisionComponent->GetScaledSphereRadius();
	const FVector Location = GetActorLocation();
	const FVector LocationSnapped = Location.GridSnap(GMapTileSize / 2);
	const bool bIsBlocked = World->SweepTestByChannel(LocationSnapped, LocationSnapped + (GMapTileSize / 2) * WantedDirection, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

	if (!bIsBlocked && FVector::Distance(Location, LocationSnapped) < 10.0f)
	{
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

void APacmanPawn::MoveToStartLocation()
{
	SetActorLocation(InitialLocation, false, nullptr, ETeleportType::ResetPhysics);

	APacmanPawn* CDO = StaticClass()->GetDefaultObject<APacmanPawn>();
	CurrentDirection = CDO->CurrentDirection;
	WantedDirection = CDO->WantedDirection;
}

void APacmanPawn::SelectRandomDirection(const FVector& Location)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const float Radius = CollisionComponent->GetScaledSphereRadius();

	const FVector Directions[] =
	{
		FVector(1.0f, 0.0f, 0.0f), FVector(-1.0f, 0.0f, 0.0f), FVector(0.0f, 1.0f, 0.0f), FVector(0.0f, -1.0f, 0.0f)
	};
	bool bIsBlocked[4] = {};
	for (uint32 Idx = 0; Idx < 4; ++Idx)
	{
		bIsBlocked[Idx] = World->SweepTestByChannel(Location, Location + (GMapTileSize / 2) * Directions[Idx], FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));
	}
	int32 SelectedDirection = 0;
	for (;;)
	{
		SelectedDirection = FMath::RandRange(0, 3);
		if (!bIsBlocked[SelectedDirection])
		{
			break;
		}
	}
	WantedDirection = Directions[SelectedDirection];
	CurrentDirection = WantedDirection;
}

#undef LOCTEXT_NAMESPACE
