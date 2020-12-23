#include "PacmanGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Blueprint/UserWidget.h"
#include "PacmanPawn.h"
#include "GhostPawn.h"
#include "GenericInfoWidget.h"

PRAGMA_DISABLE_OPTIMIZATION

#define LOCTEXT_NAMESPACE "PacmanGameModeBase"

APacmanGameModeBase::APacmanGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	DefaultPawnClass = APacmanPawn::StaticClass();

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_MainMenu"));
		check(Finder.Class);

		MainMenuWidgetClass = Finder.Class;
		MainMenuWidget = nullptr;
	}
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_PauseMenu"));
		check(Finder.Class);

		PauseMenuWidgetClass = Finder.Class;
		PauseMenuWidget = nullptr;
	}
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_GenericInfo"));
		check(Finder.Class);

		GenericInfoWidgetClass = Finder.Class;
		GenericInfoWidget = nullptr;
	}

	bIsInGameLevel = false;
	bShowInfoWidget = false;
}

void APacmanGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	check(GenericInfoWidgetClass);
	GenericInfoWidget = CastChecked<UGenericInfoWidget>(CreateWidget(GetWorld(), GenericInfoWidgetClass));

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (LevelName == TEXT("Main"))
	{
		check(MainMenuWidgetClass);

		MainMenuWidget = CreateWidget(GetWorld(), MainMenuWidgetClass);
		MainMenuWidget->AddToViewport();

		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;

		bIsInGameLevel = false;
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		Pacman = CastChecked<APacmanPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass()));

		TArray<AActor*> GhostActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostPawn::StaticClass(), GhostActors);

		for (AActor* Actor : GhostActors)
		{
			Ghosts.Add(CastChecked<AGhostPawn>(Actor));
		}

		DirectionUpdateTime = 0.0f;
		bIsInGameLevel = true;

		ShowGetReadyInfoWidget();
	}
}

void APacmanGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsInGameLevel && !bShowInfoWidget)
	{
		MovePacman(DeltaTime);
		MoveGhosts(DeltaTime);
	}
}

void APacmanGameModeBase::MovePacman(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	const float Radius = Pacman->CollisionComponent->GetScaledSphereRadius();
	const bool bIsBlocked = World->SweepTestByChannel(Pacman->GetActorLocation(), Pacman->GetActorLocation() + Pacman->WantedDirection * Radius * 0.5f, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

	if (!bIsBlocked)
	{
		Pacman->CurrentDirection = Pacman->WantedDirection;
	}

	const FVector Delta = Pacman->CurrentDirection * DeltaTime * 400.0f;

	FHitResult Hit;
	Pacman->MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		Pacman->MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
	}
}

void APacmanGameModeBase::MoveGhosts(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if ((DirectionUpdateTime += DeltaTime) > 0.25f)
	{
		DirectionUpdateTime = 0.0f;

		for (AGhostPawn* Ghost : Ghosts)
		{
			const float Radius = Ghost->CollisionComponent->GetScaledSphereRadius();
			const FVector GhostLocation = Ghost->GetActorLocation();
			const FVector GhostDirection = Ghost->CurrentDirection;

			FVector TargetLocation;
			switch (Ghost->GhostColor)
			{
			case EGhostColor::Red:
				TargetLocation = Pacman->GetActorLocation();
				//TargetLocation = FVector(950.0f, -650.0f, 50.0f);
				break;
			case EGhostColor::Pink:
				TargetLocation = Pacman->GetActorLocation() + 4 * 100.0f * Pacman->CurrentDirection;
				//TargetLocation = FVector(-750.0f, -750.0f, 50.0f);
				break;
			case EGhostColor::Blue:
				TargetLocation = Ghosts[0]->GetActorLocation() + 2.0f * ((Pacman->GetActorLocation() + 2 * 100.0f * Pacman->CurrentDirection) - Ghosts[0]->GetActorLocation());
				//TargetLocation = FVector(450.0f, 950.0f, 50.0f);
				break;
			case EGhostColor::Orange: {
				const float Distance = FVector::Distance(Pacman->GetActorLocation(), GhostLocation);
				if (Distance >= 8.0f * 100.0f)
				{
					TargetLocation = Pacman->GetActorLocation();
				}
				else
				{
					TargetLocation = FVector(-950.0f, 750.0f, 50.0f);
				}
				break;
			}
			default:
				check(false);
			}

			const float Speed = DeltaTime * Ghost->Speed;
			const FVector Destinations[3] =
			{
				GhostLocation + GhostDirection * Speed,
				GhostLocation + FVector(GhostDirection.Y, GhostDirection.X, 0.0f) * Speed,
				GhostLocation + FVector(-GhostDirection.Y, -GhostDirection.X, 0.0f) * Speed,
			};
			float Distances[3] = {};
			bool bIsBlocked[3] = {};

			for (uint32 Idx = 0; Idx < 3; ++Idx)
			{
				bIsBlocked[Idx] = World->SweepTestByChannel(GhostLocation, Destinations[Idx], FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

				Distances[Idx] = FVector::Distance(Destinations[Idx], TargetLocation);
			}

			int32 SelectedDirection = -1;
			float MinDistance = 100000.0f;
			for (uint32 Idx = 0; Idx < 3; ++Idx)
			{
				if (bIsBlocked[Idx] == false && Distances[Idx] < MinDistance)
				{
					MinDistance = Distances[Idx];
					SelectedDirection = (int32)Idx;
				}
			}

			FVector WantedDirection;

			if (SelectedDirection == 0)
			{
				WantedDirection = GhostDirection;
			}
			else if (SelectedDirection == 1)
			{
				WantedDirection = FVector(GhostDirection.Y, GhostDirection.X, 0.0f);
			}
			else if (SelectedDirection == 2)
			{
				WantedDirection = FVector(-GhostDirection.Y, -GhostDirection.X, 0.0f);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PacmanGameModeBase: All paths blocked!"));
				WantedDirection = GhostDirection;
				DirectionUpdateTime = 1.0f;
			}

			Ghost->CurrentDirection = WantedDirection;
		}
	}

	for (AGhostPawn* Ghost : Ghosts)
	{
		if (Ghost->Frozen > 0.0f)
		{
			Ghost->Frozen -= DeltaTime;
			if (Ghost->Frozen <= 0.0f)
			{
				Ghost->Frozen = 0.0f;
			}
			else
			{
				continue;
			}
		}

		const FVector Delta = Ghost->CurrentDirection * Ghost->Speed * DeltaTime;

		FHitResult Hit;
		Ghost->MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			Ghost->MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
		}
	}
}

void APacmanGameModeBase::PauseGame()
{
	check(PauseMenuWidgetClass);

	if (PauseMenuWidget == nullptr)
	{
		PauseMenuWidget = CreateWidget(GetWorld(), PauseMenuWidgetClass);
	}

	PauseMenuWidget->AddToViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(FInputModeUIOnly());
	PC->SetPause(true);
	PC->bShowMouseCursor = true;
}

void APacmanGameModeBase::ResumeGame()
{
	check(PauseMenuWidgetClass);
	check(PauseMenuWidget);

	PauseMenuWidget->RemoveFromViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(FInputModeGameOnly());
	PC->SetPause(false);
	PC->bShowMouseCursor = false;
}

void APacmanGameModeBase::BeginNewGame()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Level_01"));
}

void APacmanGameModeBase::ReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Main"));
}

void APacmanGameModeBase::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, true);
}

void APacmanGameModeBase::KillPacman()
{
	if (Pacman->Kill() == 0)
	{
		GenericInfoWidget->Text->SetText(LOCTEXT("GameOver", "Game Over"));
		GenericInfoWidget->AddToViewport();

		bShowInfoWidget = true;

		GetWorldTimerManager().SetTimer(Timer, [this](){
			GenericInfoWidget->RemoveFromViewport();
			bShowInfoWidget = false;
			ReturnToMainMenu();
		}, 2.0f, false);
	}
	else
	{
		for (AGhostPawn* Ghost : Ghosts)
		{
			Ghost->SetInitialState();
		}

		ShowGetReadyInfoWidget();
	}
}

void APacmanGameModeBase::ShowGetReadyInfoWidget()
{
	GenericInfoWidget->Text->SetText(LOCTEXT("Ready", "Get Ready!"));
	GenericInfoWidget->AddToViewport();

	bShowInfoWidget = true;

	GetWorldTimerManager().SetTimer(Timer, [this](){
		GenericInfoWidget->RemoveFromViewport();
		bShowInfoWidget = false;
	}, 2.0f, false);
}

#undef LOCTEXT_NAMESPACE
