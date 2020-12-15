#include "PacmanPawn.h"
#include "Components/SphereComponent.h"
#include "Components/TextBlock.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Blueprint/UserWidget.h"
#include "PacmanGameModeBase.h"
#include "PacmanHUDWidget.h"
#include "PacmanFood.h"

PRAGMA_DISABLE_OPTIMIZATION

#define LOCTEXT_NAMESPACE "Pacman"

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

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_HUD"));
		check(Finder.Class);
		HUDWidgetClass = Finder.Class;
	}
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

static AActor* FindActorByName(const FString& Name, UWorld* World)
{
	for (ULevel* Level : World->GetLevels())
	{
		if (Level && Level->IsCurrentLevel())
		{
			for (AActor* Actor : Level->Actors)
			{
				if (Actor && Actor->GetName() == TEXT("MainCamera"))
				{
					return Actor;
				}
			}
		}
	}
	return nullptr;
}

void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()) != TEXT("Main"))
	{
		check(HUDWidgetClass);
		HUDWidget = Cast<UPacmanHUDWidget>(CreateWidget(GetWorld(), HUDWidgetClass));
		HUDWidget->AddToViewport();

		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		check(PlayerController);

		AActor* Camera = FindActorByName(TEXT("MainCamera"), GetWorld());
		check(Camera);

		PlayerController->SetViewTarget(Camera);
	}
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

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	PlayerInputComponent->BindAction(TEXT("PauseGame"), IE_Pressed, GameMode, &APacmanGameModeBase::PauseGame);
}

UPawnMovementComponent* APacmanPawn::GetMovementComponent() const
{
	return MovementComponent;
}

void APacmanPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APacmanFood* Food = Cast<APacmanFood>(OtherActor);
	if (Food)
	{
		Score += Food->Score;
		HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), Score));
		Food->Destroy();
	}
}

#undef LOCTEXT_NAMESPACE
