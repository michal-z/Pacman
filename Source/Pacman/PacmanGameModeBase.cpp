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

constexpr float GMapTileSize = 100.0f;
constexpr float GFrightenedModeDuration = 5.0f;

APacmanGameModeBase::APacmanGameModeBase()
{
	Self.PrimaryActorTick.bCanEverTick = true;
	Self.DefaultPawnClass = APacmanPawn::StaticClass();

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_MainMenu"));
		Self.MainMenuWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_PauseMenu"));
		Self.PauseMenuWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_GenericInfo"));
		Self.GenericInfoWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> Finder(TEXT("/Game/Materials/M_GhostSuperFood"));
		Self.GhostFrightenedModeMaterial = Finder.Object;
	}
}

void APacmanGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	check(Self.GenericInfoWidgetClass);
	Self.GenericInfoWidget = CastChecked<UGenericInfoWidget>(CreateWidget(GetWorld(), Self.GenericInfoWidgetClass));

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (LevelName == TEXT("Main"))
	{
		check(Self.MainMenuWidgetClass);

		Self.MainMenuWidget = CreateWidget(GetWorld(), Self.MainMenuWidgetClass);
		Self.MainMenuWidget->AddToViewport();

		PC->SetInputMode(FInputModeUIOnly());
		PC->bShowMouseCursor = true;
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;

		Self.Pacman = CastChecked<APacmanPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass()));

		TArray<AActor*> GhostActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostPawn::StaticClass(), GhostActors);

		for (AActor* Actor : GhostActors)
		{
			Self.Ghosts.Add(CastChecked<AGhostPawn>(Actor));
		}

		Self.GameLevel = 1;
		Self.GhostModeTimer = Self.GhostModeDurations[Self.GhostModeIndex];

		ShowGetReadyInfoWidget();
	}
}

void APacmanGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Self.GameLevel > 0 && !Self.bShowInfoWidget)
	{
		Self.Pacman->Move(DeltaTime);
		MoveGhosts(DeltaTime);
	}
}

void APacmanGameModeBase::MoveGhosts(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (Self.FrightenedModeTimer > 0.0f)
	{
		Self.FrightenedModeTimer -= DeltaTime;
		if (Self.FrightenedModeTimer < 0.0f)
		{
			Self.FrightenedModeTimer = 0.0f;

			for (AGhostPawn* Ghost : Self.Ghosts)
			{
				Ghost->VisualComponent->SetMaterial(0, Ghost->DefaultMaterial);
				Ghost->bIsFrightened = false;
			}
		}
	}

	if (Self.GhostModeTimer > 0.0f && Self.FrightenedModeTimer == 0.0f)
	{
		Self.GhostModeTimer -= DeltaTime;
		if (Self.GhostModeTimer < 0.0f)
		{
			if (++Self.GhostModeIndex == _countof(Self.GhostModeDurations))
			{
				Self.GhostModeTimer = 0.0f;
			}
			else
			{
				Self.GhostModeTimer = Self.GhostModeDurations[Self.GhostModeIndex];
			}
		}
	}

	if ((Self.DirectionUpdateTimer += DeltaTime) > 0.25f)
	{
		Self.DirectionUpdateTimer = 0.0f;

		for (AGhostPawn* Ghost : Self.Ghosts)
		{
			const float Radius = Ghost->CollisionComponent->GetScaledSphereRadius();
			const FVector GhostLocation = Ghost->GetActorLocation();
			const FVector GhostDirection = Ghost->CurrentDirection;

			FVector TargetLocation;
			if (Self.FrightenedModeTimer > 0.0f) // "Frightened mode" (Ghosts choose max. distance from Pacman).
			{
				TargetLocation = Self.Pacman->GetActorLocation();
			}
			else if ((Self.GhostModeIndex % 2) == 1) // Odd GhostModeIndex is Chase mode.
			{
				switch (Ghost->Color)
				{
				case EGhostColor::Red:
					TargetLocation = Self.Pacman->GetActorLocation();
					break;
				case EGhostColor::Pink:
					TargetLocation = Self.Pacman->GetActorLocation() + 4 * GMapTileSize * Self.Pacman->CurrentDirection;
					break;
				case EGhostColor::Blue:
					{
						const FVector RedGhostLocation = Self.Ghosts[(int32)EGhostColor::Red]->GetActorLocation();
						TargetLocation = RedGhostLocation + 2.0f * ((Self.Pacman->GetActorLocation() + 2.0f * GMapTileSize * Self.Pacman->CurrentDirection) - RedGhostLocation);
						break;
					}
				case EGhostColor::Orange:
					{
						const float Distance = FVector::Distance(Self.Pacman->GetActorLocation(), GhostLocation);
						if (Distance >= 8.0f * GMapTileSize)
						{
							TargetLocation = Self.Pacman->GetActorLocation();
						}
						else
						{
							TargetLocation = Ghost->ScatterTargetLocation;
						}
						break;
					}
				}
			}
			else // Even GhostModeIndex is Scatter mode.
			{
				TargetLocation = Ghost->ScatterTargetLocation;
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
			if (Self.FrightenedModeTimer == 0.0f)
			{
				float MinDistance = 100000.0f;
				for (uint32 Idx = 0; Idx < 3; ++Idx)
				{
					if (bIsBlocked[Idx] == false && Distances[Idx] < MinDistance)
					{
						MinDistance = Distances[Idx];
						SelectedDirection = (int32)Idx;
					}
				}
			}
			else
			{
				float MaxDistance = 0.0f;
				for (uint32 Idx = 0; Idx < 3; ++Idx)
				{
					if (bIsBlocked[Idx] == false && Distances[Idx] > MaxDistance)
					{
						MaxDistance = Distances[Idx];
						SelectedDirection = (int32)Idx;
					}
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
				DirectionUpdateTimer = 1.0f;
			}

			Ghost->CurrentDirection = WantedDirection;
		}
	}

	for (AGhostPawn* Ghost : Self.Ghosts)
	{
		if (Ghost->FrozenModeTimer > 0.0f)
		{
			Ghost->FrozenModeTimer -= DeltaTime;
			if (Ghost->FrozenModeTimer <= 0.0f)
			{
				Ghost->FrozenModeTimer = 0.0f;
				if (Ghost->bIsInHouse)
				{
					Ghost->bIsInHouse = false;
					Ghost->bIsFrightened = false;
					Ghost->SetActorLocation(Ghost->SpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
					Ghost->FrozenModeTimer = 1.0f;
				}
			}
			else
			{
				continue;
			}
		}

		const float GhostSpeed = Ghost->Speed * ((Self.FrightenedModeTimer > 0.0f && Ghost->bIsFrightened) ? 0.5f : 1.0f);
		const FVector Delta = Ghost->CurrentDirection * GhostSpeed * DeltaTime;

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
	check(Self.PauseMenuWidgetClass);

	if (Self.PauseMenuWidget == nullptr)
	{
		Self.PauseMenuWidget = CreateWidget(GetWorld(), Self.PauseMenuWidgetClass);
	}

	Self.PauseMenuWidget->AddToViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PC->SetInputMode(FInputModeUIOnly());
	PC->SetPause(true);
	PC->bShowMouseCursor = true;
}

void APacmanGameModeBase::ResumeGame()
{
	check(Self.PauseMenuWidgetClass);
	check(Self.PauseMenuWidget);

	Self.PauseMenuWidget->RemoveFromViewport();

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

void APacmanGameModeBase::NotifyGhostBeginOverlap(AActor* PacmanOrGhost, AGhostPawn* InGhost)
{
	if (InGhost->bIsInHouse)
	{
		return;
	}

	if (Cast<APacmanPawn>(PacmanOrGhost)) // Pacman - Ghost overlap.
	{
		if (Self.FrightenedModeTimer > 0.0f && InGhost->bIsFrightened)
		{
			InGhost->TeleportToHouse();
			InGhost->FrozenModeTimer = 5.0f;
			Self.Pacman->Score += 100;
			return;
		}

		if (Self.Pacman->Kill() == 0)
		{
			Self.GenericInfoWidget->Text->SetText(LOCTEXT("GameOver", "Game Over"));
			Self.GenericInfoWidget->AddToViewport();

			Self.bShowInfoWidget = true;

			GetWorldTimerManager().SetTimer(Self.TimerHandle,
				[this]()
				{
					Self.GenericInfoWidget->RemoveFromViewport();
					Self.bShowInfoWidget = false;
					ReturnToMainMenu();
				},
				2.0f, false);
		}
		else
		{
			for (AGhostPawn* Ghost : Self.Ghosts)
			{
				Ghost->TeleportToHouse();
			}

			Self.GhostModeIndex = 0;
			Self.GhostModeTimer = Self.GhostModeDurations[Self.GhostModeIndex];
			Self.DirectionUpdateTimer = 0.0f;
			Self.FrightenedModeTimer = 0.0f;

			ShowGetReadyInfoWidget();
		}
	}
	else // Ghost - Ghost overlap.
	{
		AGhostPawn* OtherGhost = Cast<AGhostPawn>(PacmanOrGhost);
		if (OtherGhost && InGhost->FrozenModeTimer != 0.75f)
		{
			OtherGhost->FrozenModeTimer = 0.75f;
		}
	}
}

void APacmanGameModeBase::CompleteLevel()
{
	Self.GenericInfoWidget->Text->SetText(LOCTEXT("CompleteLevel", "You win! Congratulations!"));
	Self.GenericInfoWidget->AddToViewport();

	Self.bShowInfoWidget = true;

	GetWorldTimerManager().SetTimer(Self.TimerHandle,
		[this]()
		{
			Self.GenericInfoWidget->RemoveFromViewport();
			Self.bShowInfoWidget = false;
			ReturnToMainMenu();
		},
		2.0f, false);
}

void APacmanGameModeBase::BeginFrightenedMode()
{
	Self.FrightenedModeTimer = GFrightenedModeDuration;

	for (AGhostPawn* Ghost : Self.Ghosts)
	{
		if (Ghost->bIsInHouse == false)
		{
			Ghost->bIsFrightened = true;
			Ghost->VisualComponent->SetMaterial(0, GhostFrightenedModeMaterial);
		}
	}
}

void APacmanGameModeBase::ShowGetReadyInfoWidget()
{
	Self.GenericInfoWidget->Text->SetText(LOCTEXT("Ready", "Get Ready!"));
	Self.GenericInfoWidget->AddToViewport();

	Self.bShowInfoWidget = true;

	GetWorldTimerManager().SetTimer(Self.TimerHandle,
		[this]()
		{
			Self.GenericInfoWidget->RemoveFromViewport();
			Self.bShowInfoWidget = false;
		},
		2.0f, false);
}

#undef LOCTEXT_NAMESPACE
