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

			TeleportMaterial = UMaterialInstanceDynamic::Create(GameMode->TeleportBaseMaterial, this);
			TeleportMaterial->SetVectorParameterValue(TEXT("BaseColor"), BaseColor);
			TeleportMaterial->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
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

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode)
	{
		GameMode->NotifyGhostBeginOverlap(OtherActor, this);
	}
}

void AGhostPawn::TeleportToHouse()
{
	AGhostPawn* CDO = StaticClass()->GetDefaultObject<AGhostPawn>();

	SetActorLocation(HouseLocation, false, nullptr, ETeleportType::ResetPhysics);
	VisualComponent->SetMaterial(0, DefaultMaterial);
	CurrentDirection = CDO->CurrentDirection;
	bIsInHouse = true;
	bIsFrightened = false;
	FrozenModeTimer = LeaveHouseTime;
}
