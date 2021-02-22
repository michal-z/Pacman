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
		VisualComponent->SetStaticMesh(Finder.Object);
	}

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
	DefaultMaterial = VisualComponent->GetMaterial(0);
	{
		APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GameMode)
		{
			FLinearColor BaseColor(0.0f, 0.0f, 0.0f);
			FMaterialParameterInfo BaseColorInfo(TEXT("BaseColor"));
			DefaultMaterial->GetVectorParameterValue(BaseColorInfo, BaseColor);

			TeleportMaterial = UMaterialInstanceDynamic::Create(GameMode->GetTeleportBaseMaterial(), this);
			TeleportMaterial->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);
			TeleportMaterial->SetScalarParameterValue(TEXT("Opacity"), 1.0f);

			FrightenedModeMaterial = GameMode->GetGhostFrightenedModeMaterial();
		}
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
	VisualComponent->SetMaterial(0, DefaultMaterial);
	CurrentDirection = CDO->CurrentDirection;
	bIsInHouse = true;
	bIsFrightened = false;
	FrozenModeTimer = LeaveHouseTime;
}

float AGhostPawn::GetRadius() const
{
	return CollisionComponent->GetScaledSphereRadius();
}

void AGhostPawn::Move(float DeltaTime)
{
	if (FrozenModeTimer > 0.0f)
	{
		FrozenModeTimer -= DeltaTime;
		if (FrozenModeTimer <= 0.0f)
		{
			FrozenModeTimer = 0.0f;
			if (bIsInHouse)
			{
				bIsInHouse = false;
				bIsFrightened = false;
				FrozenModeTimer = 1.0f;
				SetActorLocation(SpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
			}
		}
		else
		{
			return;
		}
	}

	const float GhostSpeed = GetSpeed() * (bIsFrightened ? 0.5f : 1.0f);
	const FVector Delta = CurrentDirection * GhostSpeed * DeltaTime;

	FHitResult Hit;
	MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
	}
}
