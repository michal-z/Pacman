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
	This.PrimaryActorTick.bCanEverTick = false;

	This.CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	This.CollisionComponent->InitSphereRadius(49.0f);
	This.CollisionComponent->SetCollisionObjectType(ECC_Pawn);
	This.CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	This.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	This.CollisionComponent->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	This.CollisionComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	This.CollisionComponent->SetAllUseCCD(true);

	This.VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	This.VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	This.VisualComponent->SetupAttachment(This.CollisionComponent);

	This.RootComponent = This.CollisionComponent;

	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		This.VisualComponent->SetStaticMesh(Finder.Object);
	}
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> Finder(TEXT("/Game/Materials/M_Pacman"));
		This.VisualComponent->SetMaterial(0, Finder.Object);
	}

	This.MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	This.MovementComponent->UpdatedComponent = This.RootComponent;

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_HUD"));
		This.HUDWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<APacmanFood> Finder(TEXT("/Game/Blueprints/BP_SuperFood"));
		This.SuperFoodClass = Finder.Class;
	}

	This.AutoPossessPlayer = EAutoReceiveInput::Player0;

	This.CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	This.WantedDirection = This.CurrentDirection;
	This.NumLives = 3;
}

void APacmanPawn::MoveUp()
{
	This.WantedDirection = FVector(0.0f, -1.0f, 0.0f);
}

void APacmanPawn::MoveDown()
{
	This.WantedDirection = FVector(0.0f, 1.0f, 0.0f);
}

void APacmanPawn::MoveRight()
{
	This.WantedDirection = FVector(1.0f, 0.0f, 0.0f);
}

void APacmanPawn::MoveLeft()
{
	This.WantedDirection = FVector(-1.0f, 0.0f, 0.0f);
}

void APacmanPawn::BeginPlay()
{
	Super::BeginPlay();

	if (UGameplayStatics::GetCurrentLevelName(GetWorld()) != TEXT("Main"))
	{
		This.InitialLocation = GetActorLocation();

		check(This.HUDWidgetClass);
		This.HUDWidget = CastChecked<UPacmanHUDWidget>(CreateWidget(GetWorld(), This.HUDWidgetClass));
		This.HUDWidget->AddToViewport();

		This.HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), This.Score));
		This.HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), This.NumLives));

		TArray<AActor*> FoodActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APacmanFood::StaticClass(), FoodActors);
		This.NumFoodLeft = FoodActors.Num();
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
	return This.MovementComponent;
}

void APacmanPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APacmanFood* Food = Cast<APacmanFood>(OtherActor);
	if (Food)
	{
		This.Score += Food->Score;
		This.HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), This.Score));
		Food->Destroy();

		if (--This.NumFoodLeft == 0)
		{
			APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->CompleteLevel();
		}
		else if (Food->IsA(This.SuperFoodClass))
		{
			APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->BeginFrightenedMode();
		}
	}
}

uint32 APacmanPawn::Kill()
{
	check(This.NumLives > 0);

	This.NumLives -= 1;

	This.HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), This.NumLives));

	if (This.NumLives > 0)
	{
		SetActorLocation(This.InitialLocation, false, nullptr, ETeleportType::ResetPhysics);

		APacmanPawn* CDO = StaticClass()->GetDefaultObject<APacmanPawn>();
		This.CurrentDirection = CDO->CurrentDirection;
		This.WantedDirection = CDO->WantedDirection;
	}

	return This.NumLives;
}

void APacmanPawn::Move(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const float Radius = This.CollisionComponent->GetScaledSphereRadius();
	const bool bIsBlocked = World->SweepTestByChannel(GetActorLocation(), GetActorLocation() + This.WantedDirection * Radius * 0.5f, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

	if (!bIsBlocked)
	{
		This.CurrentDirection = This.WantedDirection;
	}

	const FVector Delta = This.CurrentDirection * DeltaTime * 400.0f;

	FHitResult Hit;
	This.MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		This.MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
	}
}

#undef LOCTEXT_NAMESPACE
