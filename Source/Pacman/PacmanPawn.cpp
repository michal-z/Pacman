#include "PacmanPawn.h"
#include "Components/SphereComponent.h"
#include "Components/TextBlock.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
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

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(49.0f);
	CollisionComponent->SetCollisionObjectType(ECC_Pawn);
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
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

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_HUD"));
		check(Finder.Class);
		HUDWidgetClass = Finder.Class;
	}

	CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	WantedDirection = CurrentDirection;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
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

		APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
		check(PC);

		AActor* Camera = FindActorByName(TEXT("MainCamera"), GetWorld());
		check(Camera);

		PC->SetViewTarget(Camera);
	}
}

void APacmanPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (World)
	{
		const float Radius = CollisionComponent->GetScaledSphereRadius();
		const bool bIsBlocked = World->SweepTestByChannel(GetActorLocation(), GetActorLocation() + WantedDirection * Radius * 0.5f, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

		if (!bIsBlocked)
		{
			CurrentDirection = WantedDirection;
		}
	}

	const FVector Delta = CurrentDirection * DeltaTime * 400.0f;

	FHitResult Hit;
	MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
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
