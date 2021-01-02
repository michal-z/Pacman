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
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(49.0f);
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

	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		VisualComponent->SetStaticMesh(Finder.Object);
	}
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> Finder(TEXT("/Game/Materials/M_Pacman"));
		DefaultMaterial = Finder.Object;

		VisualComponent->SetMaterial(0, DefaultMaterial);
	}

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->UpdatedComponent = RootComponent;

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_HUD"));
		HUDWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<APacmanFood> Finder(TEXT("/Game/Blueprints/BP_SuperFood"));
		SuperFoodClass = Finder.Class;
	}

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	CurrentDirection = FVector(-1.0f, 0.0f, 0.0f);
	WantedDirection = CurrentDirection;
	NumLives = 3;
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

		check(HUDWidgetClass);
		HUDWidget = CastChecked<UPacmanHUDWidget>(CreateWidget(GetWorld(), HUDWidgetClass));
		HUDWidget->AddToViewport();

		HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), Score));
		HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), NumLives));

		TArray<AActor*> FoodActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APacmanFood::StaticClass(), FoodActors);
		NumFoodLeft = FoodActors.Num();

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

		if (--NumFoodLeft == 0)
		{
			APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->CompleteLevel();
		}
		else if (Food->IsA(SuperFoodClass))
		{
			APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->BeginFrightenedMode();
		}
	}
}

uint32 APacmanPawn::Kill()
{
	check(NumLives > 0);

	NumLives -= 1;

	HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), NumLives));

	if (NumLives > 0)
	{
		APacmanPawn* CDO = StaticClass()->GetDefaultObject<APacmanPawn>();
		CurrentDirection = CDO->CurrentDirection;
		WantedDirection = CDO->WantedDirection;
	}

	return NumLives;
}

void APacmanPawn::Move(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const float Radius = CollisionComponent->GetScaledSphereRadius();
	const bool bIsBlocked = World->SweepTestByChannel(GetActorLocation(), GetActorLocation() + WantedDirection * Radius * 0.5f, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

	if (!bIsBlocked)
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

#undef LOCTEXT_NAMESPACE
