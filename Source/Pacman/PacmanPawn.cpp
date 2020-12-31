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

#define LOCTEXT_NAMESPACE "PacmanPawn"

APacmanPawn::APacmanPawn()
{
	Self.PrimaryActorTick.bCanEverTick = false;

	Self.CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	Self.CollisionComponent->InitSphereRadius(49.0f);
	Self.CollisionComponent->SetCollisionObjectType(ECC_Pawn);
	Self.CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	Self.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	Self.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Self.CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	Self.CollisionComponent->SetAllUseCCD(true);

	Self.VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	Self.VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Self.VisualComponent->SetupAttachment(Self.CollisionComponent);

	Self.RootComponent = Self.CollisionComponent;

	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		Self.VisualComponent->SetStaticMesh(Finder.Object);
	}
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> Finder(TEXT("/Game/Materials/M_Pacman"));
		Self.VisualComponent->SetMaterial(0, Finder.Object);
	}

	Self.MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	Self.MovementComponent->UpdatedComponent = Self.RootComponent;

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_HUD"));
		Self.HUDWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<APacmanFood> Finder(TEXT("/Game/Blueprints/BP_SuperFood"));
		Self.SuperFoodClass = Finder.Class;
	}

	Self.AutoPossessPlayer = EAutoReceiveInput::Player0;

	Self.CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	Self.WantedDirection = Self.CurrentDirection;
	Self.NumLives = 3;
}

void APacmanPawn::MoveUp()
{
	Self.WantedDirection = FVector(0.0f, -1.0f, 0.0f);
}

void APacmanPawn::MoveDown()
{
	Self.WantedDirection = FVector(0.0f, 1.0f, 0.0f);
}

void APacmanPawn::MoveRight()
{
	Self.WantedDirection = FVector(1.0f, 0.0f, 0.0f);
}

void APacmanPawn::MoveLeft()
{
	Self.WantedDirection = FVector(-1.0f, 0.0f, 0.0f);
}

void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()) != TEXT("Main"))
	{
		Self.InitialLocation = GetActorLocation();

		check(Self.HUDWidgetClass);
		Self.HUDWidget = CastChecked<UPacmanHUDWidget>(CreateWidget(GetWorld(), HUDWidgetClass));
		Self.HUDWidget->AddToViewport();

		Self.HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), Score));
		Self.HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), NumLives));

		TArray<AActor*> FoodActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APacmanFood::StaticClass(), FoodActors);
		Self.NumFoodLeft = FoodActors.Num();
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
}

UPawnMovementComponent* APacmanPawn::GetMovementComponent() const
{
	return Self.MovementComponent;
}

void APacmanPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APacmanFood* Food = Cast<APacmanFood>(OtherActor);
	if (Food)
	{
		Self.Score += Food->Score;
		Self.HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), Score));
		Food->Destroy();

		if (--Self.NumFoodLeft == 0)
		{
			APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->CompleteLevel();
		}
		else if (Food->IsA(Self.SuperFoodClass))
		{
			APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->BeginFrightenedMode();
		}
	}
}

uint32 APacmanPawn::Kill()
{
	check(Self.NumLives > 0);

	Self.NumLives -= 1;

	Self.HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), Self.NumLives));

	if (Self.NumLives > 0)
	{
		SetActorLocation(Self.InitialLocation, false, nullptr, ETeleportType::ResetPhysics);

		APacmanPawn* CDO = StaticClass()->GetDefaultObject<APacmanPawn>();
		Self.CurrentDirection = CDO->CurrentDirection;
		Self.WantedDirection = CDO->WantedDirection;
	}

	return Self.NumLives;
}

void APacmanPawn::Move(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const float Radius = Self.CollisionComponent->GetScaledSphereRadius();
	const bool bIsBlocked = World->SweepTestByChannel(GetActorLocation(), GetActorLocation() + WantedDirection * Radius * 0.5f, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

	if (!bIsBlocked)
	{
		Self.CurrentDirection = Self.WantedDirection;
	}

	const FVector Delta = Self.CurrentDirection * DeltaTime * 400.0f;

	FHitResult Hit;
	Self.MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		Self.MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
	}
}

#undef LOCTEXT_NAMESPACE
