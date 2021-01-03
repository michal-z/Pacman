#include "PacmanGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Blueprint/UserWidget.h"
#include "PacmanPawn.h"
#include "GhostPawn.h"
#include "PacmanWidgets.h"

PRAGMA_DISABLE_OPTIMIZATION

#define LOCTEXT_NAMESPACE "PacmanGameModeBase"

static constexpr float GMapTileSize = 100.0f;
static constexpr float GFrightenedModeDuration = 5.0f;

APacmanGameModeBase::APacmanGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;
	DefaultPawnClass = APacmanPawn::StaticClass();

	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_MainMenu"));
		MainMenuWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_PauseMenu"));
		PauseMenuWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_GenericInfo"));
		GenericInfoWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInstance> Finder(TEXT("/Game/Materials/M_GhostSuperFood"));
		GhostFrightenedModeMaterial = Finder.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UMaterial> Finder(TEXT("/Game/Materials/M_TeleportBase"));
		TeleportBaseMaterial = Finder.Object;
	}
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

		GhostModeTimer = GhostModeDurations[GhostModeIndex];
		GameLevel = 1;

		ShowGetReadyInfoWidget();
	}
}

void APacmanGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Teleport.Material)
	{
		Teleport.Opacity += Teleport.Sign * DeltaTime;
		Teleport.Material->SetScalarParameterValue(TEXT("Opacity"), FMath::Clamp(Teleport.Opacity, 0.0f, 1.0f));

		if (Teleport.Opacity <= 0.0f)
		{
			Teleport.Opacity = 0.0f;
			Teleport.Sign = -Teleport.Sign;
			Teleport.CalledWhenOpacity0();
		}
		else if (Teleport.Opacity >= 1.0f)
		{
			Teleport.CalledWhenOpacity1();
			Teleport = {};
		}
	}
	else if (GameLevel > 0 && !bShowInfoWidget)
	{
		Pacman->Move(DeltaTime);
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

	if (FrightenedModeTimer > 0.0f)
	{
		FrightenedModeTimer -= DeltaTime;
		if (FrightenedModeTimer < 0.0f)
		{
			FrightenedModeTimer = 0.0f;

			for (AGhostPawn* Ghost : Ghosts)
			{
				Ghost->DisableFrightenedMode();
			}
		}
	}

	if (GhostModeTimer > 0.0f && FrightenedModeTimer == 0.0f)
	{
		GhostModeTimer -= DeltaTime;
		if (GhostModeTimer < 0.0f)
		{
			if (++GhostModeIndex == _countof(GhostModeDurations))
			{
				GhostModeTimer = 0.0f;
			}
			else
			{
				GhostModeTimer = GhostModeDurations[GhostModeIndex];
			}
		}
	}

	if ((DirectionUpdateTimer += DeltaTime) > 0.25f)
	{
		DirectionUpdateTimer = 0.0f;

		for (AGhostPawn* Ghost : Ghosts)
		{
			const float Radius = Ghost->GetRadius();
			const FVector GhostLocation = Ghost->GetActorLocation();
			const FVector GhostDirection = Ghost->CurrentDirection;

			FVector TargetLocation;
			if (FrightenedModeTimer > 0.0f) // "Frightened mode" (Ghosts choose max. distance from Pacman).
			{
				TargetLocation = Pacman->GetActorLocation();
			}
			else if ((GhostModeIndex & 0x1) == 1) // Odd GhostModeIndex is Chase mode.
			{
				if (Ghost->GetColor() == EGhostColor::Red)
				{
					TargetLocation = Pacman->GetActorLocation();
				}
				else if (Ghost->GetColor() == EGhostColor::Pink)
				{
					TargetLocation = Pacman->GetActorLocation() + 4 * GMapTileSize * Pacman->GetCurrentDirection();
				}
				else if (Ghost->GetColor() == EGhostColor::Blue)
				{
					const FVector RedGhostLocation = Ghosts[(int32)EGhostColor::Red]->GetActorLocation();
					TargetLocation = RedGhostLocation + 2.0f * ((Pacman->GetActorLocation() + 2.0f * GMapTileSize * Pacman->GetCurrentDirection()) - RedGhostLocation);
				}
				else if (Ghost->GetColor() == EGhostColor::Orange)
				{
					const float Distance = FVector::Distance(Pacman->GetActorLocation(), GhostLocation);
					if (Distance >= 8.0f * GMapTileSize)
					{
						TargetLocation = Pacman->GetActorLocation();
					}
					else
					{
						TargetLocation = Ghost->GetScatterTargetLocation();
					}
				}
				else
				{
					check(false);
				}
			}
			else // Even GhostModeIndex is Scatter mode.
			{
				TargetLocation = Ghost->GetScatterTargetLocation();
			}

			const float Speed = DeltaTime * Ghost->GetSpeed();
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
			if (FrightenedModeTimer == 0.0f)
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

	for (AGhostPawn* Ghost : Ghosts)
	{
		Ghost->Move(DeltaTime);
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

void APacmanGameModeBase::NotifyGhostBeginOverlap(AActor* PacmanOrGhost, AGhostPawn* InGhost)
{
	if (Cast<APacmanPawn>(PacmanOrGhost)) // Pacman - Ghost overlap.
	{
		if (FrightenedModeTimer > 0.0f && InGhost->IsFrightened())
		{
			InGhost->SetInHouseState();
			InGhost->FrozenModeTimer = 5.0f;
			Pacman->AddScore(100);
			return;
		}

		if (Pacman->Kill() == 0)
		{
			GenericInfoWidget->Text->SetText(LOCTEXT("GameOver", "Game Over"));
			GenericInfoWidget->AddToViewport();

			bShowInfoWidget = true;

			GetWorldTimerManager().SetTimer(TimerHandle,
				[this]()
				{
					GenericInfoWidget->RemoveFromViewport();
					bShowInfoWidget = false;
					ReturnToMainMenu();
				},
				2.0f, false);
		}
		else
		{
			Pacman->SetTeleportMaterial();

			Teleport =
			{
				Pacman->GetTeleportMaterial(), 1.0f, -1.0f,
				[this]()
				{
					Pacman->SetActorLocation(Pacman->GetInitialLocation(), false, nullptr, ETeleportType::ResetPhysics);
					for (AGhostPawn* Ghost : Ghosts)
					{
						Ghost->SetInHouseState();
					}
				},
				[this]()
				{
					Pacman->SetDefaultMaterial();
					ShowGetReadyInfoWidget();
				}
			};

			GhostModeIndex = 0;
			GhostModeTimer = GhostModeDurations[GhostModeIndex];
			DirectionUpdateTimer = 0.0f;
			FrightenedModeTimer = 0.0f;
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
	GenericInfoWidget->Text->SetText(LOCTEXT("CompleteLevel", "You win! Congratulations!"));
	GenericInfoWidget->AddToViewport();

	bShowInfoWidget = true;

	GetWorldTimerManager().SetTimer(TimerHandle,
		[this]()
		{
			GenericInfoWidget->RemoveFromViewport();
			bShowInfoWidget = false;
			ReturnToMainMenu();
		},
		2.0f, false);
}

void APacmanGameModeBase::BeginFrightenedMode()
{
	FrightenedModeTimer = GFrightenedModeDuration;

	for (AGhostPawn* Ghost : Ghosts)
	{
		Ghost->EnableFrightenedMode();
	}
}

void APacmanGameModeBase::ShowGetReadyInfoWidget()
{
	GenericInfoWidget->Text->SetText(LOCTEXT("Ready", "Get Ready!"));
	GenericInfoWidget->AddToViewport();

	bShowInfoWidget = true;

	GetWorldTimerManager().SetTimer(TimerHandle,
		[this]()
		{
			GenericInfoWidget->RemoveFromViewport();
			bShowInfoWidget = false;
		},
		2.0f, false);
}


#include "Modules/ModuleManager.h"
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Pacman, "Pacman");

#undef LOCTEXT_NAMESPACE
