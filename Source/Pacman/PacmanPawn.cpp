#include "PacmanPawn.h"
#include "Components/SphereComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/FloatingPawnMovement.h"

PRAGMA_DISABLE_OPTIMIZATION

APacmanPawn::APacmanPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	auto CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetupAttachment(RootComponent);
	CollisionComponent->InitSphereRadius(49.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Pawn"));

	auto VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	VisualComponent->SetupAttachment(CollisionComponent);

	RootComponent = CollisionComponent;


	static ConstructorHelpers::FObjectFinder<UStaticMesh> VisualAsset(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (VisualAsset.Succeeded())
	{
		VisualComponent->SetStaticMesh(VisualAsset.Object);
	}

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	InputVector = FVector(-1.0f, 0.0f, 0.0f);
	NonBlockingInputVector = FVector::ZeroVector;
	WantedInputVector = FVector::ZeroVector;
}

void APacmanPawn::MoveUp()
{
	WantedInputVector = FVector(0.0f, -1.0f, 0.0f);
}

void APacmanPawn::MoveDown()
{
	WantedInputVector = FVector(0.0f, 1.0f, 0.0f);
}

void APacmanPawn::MoveRight()
{
	WantedInputVector = FVector(1.0f, 0.0f, 0.0f);
}

void APacmanPawn::MoveLeft()
{
	WantedInputVector = FVector(-1.0f, 0.0f, 0.0f);
}

void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();
}

void APacmanPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("(%f %f)"), InputVector.X, InputVector.Y);

	{
		const FVector Delta = InputVector * DeltaTime * 500.0f;

		FHitResult Hit;
		MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);

			if (!NonBlockingInputVector.IsZero())
			{
				InputVector = NonBlockingInputVector;
			}
			else if (!WantedInputVector.IsZero())
			{
				InputVector = WantedInputVector;
			}

			NonBlockingInputVector = FVector::ZeroVector;
		}
		else
		{
			NonBlockingInputVector = InputVector;

			if (!WantedInputVector.IsZero())
			{
				InputVector = WantedInputVector;
			}
		}
	}
}

void APacmanPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("MoveUp"), IE_Pressed, this, &APacmanPawn::MoveUp);
	PlayerInputComponent->BindAction(TEXT("MoveDown"), IE_Pressed, this, &APacmanPawn::MoveDown);
	PlayerInputComponent->BindAction(TEXT("MoveRight"), IE_Pressed, this, &APacmanPawn::MoveRight);
	PlayerInputComponent->BindAction(TEXT("MoveLeft"), IE_Pressed, this, &APacmanPawn::MoveLeft);
}

UPawnMovementComponent* APacmanPawn::GetMovementComponent() const
{
	return MovementComponent;
}
