#include "PacmanGameModeBase.h"
#include "PacmanPawn.h"
#include "PacmanGhostPawn.h"
#include "PacmanMiscClasses.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/SphereComponent.h"
#include "Components/VerticalBox.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Blueprint/UserWidget.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

PRAGMA_DISABLE_OPTIMIZATION

#define LOCTEXT_NAMESPACE "PacmanGameModeBase"

static constexpr float GFrightenedModeDuration = 5.0f;
static constexpr float GPowerUpLiveDuration = 5.0f;
static constexpr float GPowerUpSpawnPeriod = 30.0f;
static constexpr int32 GNumHiscoreEntries = 9;
static constexpr int32 GMapTileNumX = 20;
static constexpr int32 GMapTileNumY = 20;

static uint32 GPacmanNumLives;
static uint32 GPacmanScore;
static uint32 GPacmanNumRandomTeleports;
static uint32 GGameLevel;

APacmanGameModeBase::APacmanGameModeBase()
{
	PrimaryActorTick.bCanEverTick = true;

	{
		static ConstructorHelpers::FClassFinder<APacmanPawn> Finder(TEXT("/Game/Blueprints/BP_PacmanPawn"));
		DefaultPawnClass = Finder.Class;
	}
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
		static ConstructorHelpers::FClassFinder<UUserWidget> Finder(TEXT("/Game/UI/WBP_HUD"));
		HUDWidgetClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<APacmanFood> Finder(TEXT("/Game/Blueprints/BP_SuperFood"));
		SuperFoodClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FClassFinder<APowerUpTrigger> Finder(TEXT("/Game/Blueprints/BP_PowerUpTrigger"));
		PowerUpTriggerClass = Finder.Class;
	}
	{
		static ConstructorHelpers::FObjectFinder<UNiagaraSystem> Finder(TEXT("/Game/FX/NS_SuperFood"));
		SuperFoodFX = Finder.Object;
	}
	{
		static ConstructorHelpers::FObjectFinder<UNiagaraSystem> Finder(TEXT("/Game/FX/NS_PowerUp"));
		PowerUpFX = Finder.Object;
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

		MainMenuWidget = CastChecked<UMainMenuWidget>(CreateWidget(GetWorld(), MainMenuWidgetClass));
		MainMenuWidget->AddToViewport();

		UPacmanHiscore* LoadedHiscore = Cast<UPacmanHiscore>(UGameplayStatics::LoadGameFromSlot(TEXT("Hiscore"), 0));
		for (uint32 SlotIdx = 0; SlotIdx < GNumHiscoreEntries; ++SlotIdx)
		{
			auto Widget = NewObject<UTextBlock>(this);
			if (LoadedHiscore && LoadedHiscore->Entries.IsValidIndex(SlotIdx))
			{
				const auto& Entry = LoadedHiscore->Entries[SlotIdx];
				const auto Text = FText::Format(FText::FromString(TEXT("{0}. {1}: {2}")), SlotIdx + 1, Entry.Name, FText::AsNumber(Entry.Score));
				Widget->SetText(Text);
				MainMenuWidget->HiscoreBox->AddChildToVerticalBox(Widget);
			}
			else
			{
				const auto Text = FText::Format(FText::FromString(TEXT("{0}. --- Empty Slot ---")), SlotIdx + 1);
				Widget->SetText(Text);
				MainMenuWidget->HiscoreBox->AddChildToVerticalBox(Widget);
			}
		}

		if (PC)
		{
			PC->SetInputMode(FInputModeUIOnly());
			PC->bShowMouseCursor = true;
		}

		GGameLevel = 0;
		GPacmanScore = 0;
		GPacmanNumLives = 3;
		GPacmanNumRandomTeleports = 2;
	}
	else
	{
		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}

		Pacman = CastChecked<APacmanPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass()));

		TArray<AActor*> GhostActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostPawn::StaticClass(), GhostActors);

		for (AActor* Actor : GhostActors)
		{
			Ghosts.Add(CastChecked<AGhostPawn>(Actor));
		}
		Ghosts.StableSort([](const AGhostPawn& G0, const AGhostPawn& G1) { return (int32)G0.Color < (int32)G1.Color; });

		DirectionUpdateTimer = 10000.0f;
		PowerUpTimer = GPowerUpSpawnPeriod;
		GhostModeTimer = GhostModeDurations[GhostModeIndex];
		GGameLevel += 1;

		check(HUDWidgetClass);
		HUDWidget = CastChecked<UPacmanHUDWidget>(CreateWidget(GetWorld(), HUDWidgetClass));
		HUDWidget->AddToViewport();
		HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), GPacmanScore));
		HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), GPacmanNumLives));
		HUDWidget->RandomTeleportsText->SetText(FText::Format(LOCTEXT("RandomTeleports", "Teleports: {0}"), GPacmanNumRandomTeleports));
		HUDWidget->LevelText->SetText(FText::Format(LOCTEXT("Level", "Level: {0}"), GGameLevel));

		UPacmanHiscore* LoadedHiscore = Cast<UPacmanHiscore>(UGameplayStatics::LoadGameFromSlot(TEXT("Hiscore"), 0));
		if (LoadedHiscore && LoadedHiscore->Entries.Num() == GNumHiscoreEntries)
		{
			HUDWidget->HiscoreText->SetText(FText::Format(LOCTEXT("Hiscore", "Hiscore: {0}"), LoadedHiscore->Entries.Last().Score));
		}
		else
		{
			HUDWidget->HiscoreText->SetText(LOCTEXT("Hiscore", "Hiscore: ---"));
		}

		TArray<AActor*> FoodActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APacmanFood::StaticClass(), FoodActors);
		NumFoodLeft = FoodActors.Num();

		ShowGetReadyInfoWidget();
	}
}

void APacmanGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		return;
	}

	for (uint32 Idx = 0; Idx < _countof(Teleports); ++Idx)
	{
		FTeleport& Teleport = Teleports[Idx];
		if (!Teleport.Material)
		{
			continue;
		}

		const float MaxOpacity = Idx == ((int32)EGhostColor::Orange + 1) ? 1.0f : GGhostDefaultOpacity;

		Teleport.Opacity += Teleport.Sign * DeltaTime;
		Teleport.Material->SetScalarParameterValue(TEXT("Opacity"), FMath::Clamp(Teleport.Opacity, 0.0f, MaxOpacity));

		if (Teleport.Opacity < 0.0f)
		{
			Teleport.Opacity = 0.0f;
			Teleport.Sign = -Teleport.Sign;
			if (Teleport.CalledWhenOpacity0)
			{
				Teleport.CalledWhenOpacity0();
			}
			Teleport.Material->SetScalarParameterValue(TEXT("Opacity"), 0.0f);
		}
		else if (Teleport.Opacity > MaxOpacity)
		{
			//if (Teleport.CalledWhenOpacity1)
			//{
				//Teleport.CalledWhenOpacity1();
			//}
			Teleport.Material->SetScalarParameterValue(TEXT("Opacity"), MaxOpacity);
			Teleport = {};
		}
	}

	if (Teleports[(int32)EGhostColor::Orange + 1].Material)
	{
		// Pacman teleport is in progress - so, block ("pause") game.
		return;
	}

	if (GGameLevel > 0 && !GenericInfoWidget->IsInViewport())
	{
		if (PowerUpTimer > 0.0f)
		{
			PowerUpTimer -= DeltaTime;
			if (PowerUpTimer < 0.0f)
			{
				if (CurrentPowerUp)
				{
					CurrentPowerUp->Destroy();
					CurrentPowerUp = nullptr;
					PowerUpTimer = GPowerUpSpawnPeriod;
				}
				else
				{
					UWorld* World = GetWorld();
					if (World)
					{
						const FVector RandomLocation = SelectRandomLocationOnMap(World, CurrentPowerUpLocation);
						CurrentPowerUp = World->SpawnActor(PowerUpTriggerClass, &RandomLocation, &FRotator::ZeroRotator);
						if (CurrentPowerUp)
						{
							CurrentPowerUpLocation = CurrentPowerUp->GetActorLocation();
						}
						PowerUpTimer = GPowerUpLiveDuration;
						//UNiagaraFunctionLibrary::SpawnSystemAttached(PowerUpFX, CurrentPowerUp->CollisionComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
					}
				}
			}
		}

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
				Ghost->bIsFrightened = false;
				FTeleport& Teleport = Teleports[(int32)Ghost->Color];
				if (Teleport.Material != nullptr)
				{
					Ghost->FrightenedMaterial->SetScalarParameterValue(TEXT("Opacity"), GGhostDefaultOpacity);
					Teleport.Material = Ghost->Material;
					Teleport.Material->SetScalarParameterValue(TEXT("Opacity"), Teleport.Opacity);
				}
				Ghost->SetDefaultMaterial();
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
			const float Radius = GMapTileSize / 2 - 1.0f;
			const FVector GhostDirection = Ghost->CurrentDirection;
			const FVector GhostLocationSnapped = Ghost->GetActorLocation().GridSnap(GMapTileSize / 2);
			const FVector PacmanLocationSnapped = Pacman->GetActorLocation().GridSnap(GMapTileSize / 2);

			FVector TargetLocation;
			if (FrightenedModeTimer > 0.0f) // "Frightened mode" (Ghosts choose max. distance from Pacman).
			{
				TargetLocation = PacmanLocationSnapped;
			}
			else if ((GhostModeIndex & 0x1) == 1) // Odd GhostModeIndex is Chase mode.
			{
				if (Ghost->Color == EGhostColor::Red)
				{
					TargetLocation = PacmanLocationSnapped;
				}
				else if (Ghost->Color == EGhostColor::Pink)
				{
					TargetLocation = PacmanLocationSnapped + 4 * GMapTileSize * Pacman->CurrentDirection;
				}
				else if (Ghost->Color == EGhostColor::Blue)
				{
					const FVector RedGhostLocation = Ghosts[(int32)EGhostColor::Red]->GetActorLocation().GridSnap(GMapTileSize / 2);
					TargetLocation = RedGhostLocation + 2.0f * ((PacmanLocationSnapped + 2.0f * GMapTileSize * Pacman->CurrentDirection) - RedGhostLocation);
				}
				else if (Ghost->Color == EGhostColor::Orange)
				{
					const float Distance = FVector::Distance(PacmanLocationSnapped, GhostLocationSnapped);
					if (Distance >= 8.0f * GMapTileSize)
					{
						TargetLocation = PacmanLocationSnapped;
					}
					else
					{
						TargetLocation = Ghost->ScatterTargetLocation;
					}
				}
				else
				{
					check(false);
				}
			}
			else // Even GhostModeIndex is Scatter mode.
			{
				TargetLocation = Ghost->ScatterTargetLocation;
			}

			const FVector Directions[] =
			{
				GhostDirection,
				FVector(GhostDirection.Y, GhostDirection.X, 0.0f),
				FVector(-GhostDirection.Y, -GhostDirection.X, 0.0f),
				FVector(-GhostDirection.X, -GhostDirection.Y, 0.0f),
			};
			float Distances[4] = {};
			bool bIsBlocked[4] = {};

			for (uint32 Idx = 0; Idx < 4; ++Idx)
			{
				bIsBlocked[Idx] = World->SweepTestByChannel(
					GhostLocationSnapped,
					GhostLocationSnapped + (GMapTileSize / 2) * Directions[Idx],
					FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

				Distances[Idx] = FVector::Distance(GhostLocationSnapped + (GMapTileSize / 2) * Directions[Idx], TargetLocation);
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
				float MaxDistance = -1.0f;
				for (uint32 Idx = 0; Idx < 3; ++Idx)
				{
					if (bIsBlocked[Idx] == false && Distances[Idx] > MaxDistance)
					{
						MaxDistance = Distances[Idx];
						SelectedDirection = (int32)Idx;
					}
				}
				// At the beginning of 'Frightened Mode' ghosts can choose backward direction.
				if (FrightenedModeTimer > (GFrightenedModeDuration - 0.5f))
				{
					if (bIsBlocked[3] == false && Distances[3] > MaxDistance)
					{
						SelectedDirection = 3;
					}
				}
			}

			check(SelectedDirection >= 0 && SelectedDirection < _countof(Directions));
			Ghost->CurrentDirection = Directions[SelectedDirection];
		}
	}

	for (AGhostPawn* Ghost : Ghosts)
	{
		if (Ghost->FrozenModeTimer > 0.0f)
		{
			Ghost->FrozenModeTimer -= DeltaTime;
			if (Ghost->FrozenModeTimer <= 0.0f)
			{
				Ghost->FrozenModeTimer = 0.0f;
				if (Ghost->bIsInHouse)
				{
					Ghost->FrozenModeTimer = 1.5f;
					Ghost->Material->SetScalarParameterValue(TEXT("Opacity"), GGhostDefaultOpacity);
					Ghost->SetDefaultMaterial();
					Teleports[(int32)Ghost->Color] =
					{
						Ghost->Material, GGhostDefaultOpacity, -1.0f,
						[this, Ghost]()
						{
							Ghost->SetActorLocation(Ghost->SpawnLocation, false, nullptr, ETeleportType::ResetPhysics);
							Ghost->bIsInHouse = false;
							Ghost->bIsFrightened = false;
							Ghost->FrozenModeTimer = 1.5f;
						},
					};
					continue;
				}
			}
			else
			{
				continue;
			}
		}
		Ghost->Move(DeltaTime);
	}
}

void APacmanGameModeBase::SaveHiscoreName(const FText& PlayerName, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter)
	{
		return;
	}
	UPacmanHiscore* LoadedHiscore = Cast<UPacmanHiscore>(UGameplayStatics::LoadGameFromSlot(TEXT("Hiscore"), 0));
	UPacmanHiscore* NewHiscore = Cast<UPacmanHiscore>(UGameplayStatics::CreateSaveGameObject(UPacmanHiscore::StaticClass()));
	if (LoadedHiscore)
	{
		NewHiscore->Entries = LoadedHiscore->Entries;
	}
	if (PlayerName.IsEmptyOrWhitespace())
	{
		NewHiscore->Entries.Add({ LOCTEXT("EmptyName", "Unnamed"), GPacmanScore });
	}
	else
	{
		NewHiscore->Entries.Add({ PlayerName, GPacmanScore });
	}
	NewHiscore->Entries.StableSort(TGreater<FHiscoreEntry>());
	const auto Size = NewHiscore->Entries.Num();
	if (Size > GNumHiscoreEntries)
	{
		NewHiscore->Entries.RemoveAt(Size - 1, Size - GNumHiscoreEntries);
	}
	UGameplayStatics::SaveGameToSlot(NewHiscore, TEXT("Hiscore"), 0);

	GenericInfoWidget->RemoveFromViewport();
	GetWorldTimerManager().ClearTimer(TimerHandle);
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Main"));
}

void APacmanGameModeBase::PauseGame()
{
	check(PauseMenuWidgetClass);

	if (PauseMenuWidget == nullptr)
	{
		PauseMenuWidget = CreateWidget(GetWorld(), PauseMenuWidgetClass);
	}

	if (PauseMenuWidget->IsInViewport())
	{
		ResumeGame();
		return;
	}

	PauseMenuWidget->AddToViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
		PC->bShowMouseCursor = true;
	}

	if (GetWorldTimerManager().IsTimerActive(TimerHandle))
	{
		GetWorldTimerManager().PauseTimer(TimerHandle);
	}
}

void APacmanGameModeBase::ResumeGame()
{
	check(PauseMenuWidgetClass);
	check(PauseMenuWidget);

	PauseMenuWidget->RemoveFromViewport();

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	if (GetWorldTimerManager().IsTimerPaused(TimerHandle))
	{
		GetWorldTimerManager().UnPauseTimer(TimerHandle);
	}
}

void APacmanGameModeBase::NewGame()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Level_01"));
}

void APacmanGameModeBase::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, true);
}

void APacmanGameModeBase::ReturnToMainMenu()
{
	GetWorldTimerManager().ClearTimer(TimerHandle);
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Main"));
}

uint32 APacmanGameModeBase::KillPacman()
{
	if (GPacmanNumLives == 0 || GPacmanNumLives == 1)
	{
		return GPacmanNumLives = 0;
	}

	GPacmanNumLives -= 1;
	HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), GPacmanNumLives));
	return GPacmanNumLives;
}

void APacmanGameModeBase::HandleActorOverlap(AActor* PacmanOrGhost, AActor* Other)
{
	if ((PacmanOrGhost && PacmanOrGhost->IsPendingKill()) || (Other && Other->IsPendingKill()))
	{
		return;
	}

	APacmanPawn* PacmanPawn = Cast<APacmanPawn>(PacmanOrGhost);
	APacmanFood* PacmanFood = Cast<APacmanFood>(Other);
	AGhostPawn* GhostPawn = Cast<AGhostPawn>(Other);
	APowerUpTrigger* PowerUpTrigger = Cast<APowerUpTrigger>(Other);

	if (PacmanPawn && PacmanFood && NumFoodLeft > 0) // Pacman - "Food" overlap.
	{
		GPacmanScore += PacmanFood->GetScore();
		HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), GPacmanScore));

		if (--NumFoodLeft == 0)
		{
			CompleteLevel();
		}
		else if (PacmanFood->IsA(SuperFoodClass))
		{
			BeginFrightenedMode();
			//UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SuperFoodFX, PacmanFood->GetActorLocation());
		}

		PacmanFood->Destroy();
	}
	else if (PacmanPawn && GhostPawn && GPacmanNumLives > 0) // Pacman - Ghost overlap.
	{
		if (Teleports[(int32)GhostPawn->Color].Material != nullptr)
		{
			// Ghost has been already killed and is teleporting - just ignore.
			return;
		}

		if (FrightenedModeTimer > 0.0f && GhostPawn->bIsFrightened)
		{
			//UNiagaraComponent* FX = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SuperFoodFX, GhostPawn->GetActorLocation());
			{
				FLinearColor Color;
				GhostPawn->Material->GetVectorParameterDefaultValue(FHashedMaterialParameterInfo(TEXT("BaseColor")), Color);
				//FX->SetVariableLinearColor(TEXT("Color"), Color);
			}

			GhostPawn->FrozenModeTimer = 5.0f;
			GPacmanScore += 100;
			HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), GPacmanScore));

			GhostPawn->Material->SetScalarParameterValue(TEXT("Opacity"), GGhostDefaultOpacity);
			GhostPawn->SetDefaultMaterial();
			Teleports[(int32)GhostPawn->Color] =
			{
				GhostPawn->Material, GGhostDefaultOpacity, -1.0f,
				[this, GhostPawn]()
				{
					GhostPawn->MoveToGhostHouse();
				},
			};
			return;
		}

		if (KillPacman() == 0) // Pacman lost all lives.
		{
			UPacmanHiscore* LoadedHiscore = Cast<UPacmanHiscore>(UGameplayStatics::LoadGameFromSlot(TEXT("Hiscore"), 0));
			if (LoadedHiscore == nullptr
				|| (LoadedHiscore && GPacmanScore > LoadedHiscore->Entries.Last().Score)
				|| (LoadedHiscore && LoadedHiscore->Entries.Num() < GNumHiscoreEntries))
			{
				GetWorldTimerManager().ClearTimer(TimerHandle);
				GenericInfoWidget->Text->SetText(LOCTEXT("EnterName", "Type your name and press <Enter>"));
				GenericInfoWidget->AddToViewport();
				GenericInfoWidget->PlayerName->SetVisibility(UWidget::ConvertRuntimeToSerializedVisibility(EVisibility::Visible));
				GenericInfoWidget->PlayerName->SetFocus();

				APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
				if (PC)
				{
					PC->SetInputMode(FInputModeUIOnly());
					PC->bShowMouseCursor = true;
				}
			}
			else
			{
				GenericInfoWidget->Text->SetText(LOCTEXT("GameOver", "Game Over"));
				GenericInfoWidget->AddToViewport();
				GetWorldTimerManager().SetTimer(TimerHandle,
					[this]()
					{
						GenericInfoWidget->RemoveFromViewport();
						UGameplayStatics::OpenLevel(GetWorld(), TEXT("Main"));
					},
					2.0f, false);
			}
		}
		else
		{
			Pacman->Material->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
			Teleports[(int32)EGhostColor::Orange + 1] =
			{
				Pacman->Material, 1.0f, -1.0f,
				[this]()
				{
					Pacman->MoveToStartLocation();
					ShowGetReadyInfoWidget();
					for (AGhostPawn* Ghost : Ghosts)
					{
						Ghost->MoveToGhostHouse();
					}
				},
			};

			GhostModeIndex = 0;
			GhostModeTimer = GhostModeDurations[GhostModeIndex];
			PowerUpTimer = GPowerUpSpawnPeriod;
			DirectionUpdateTimer = 10000.0f;
			FrightenedModeTimer = 0.0f;
			if (CurrentPowerUp)
			{
				CurrentPowerUp->Destroy();
				CurrentPowerUp = nullptr;
			}
		}
	}
	else if (PacmanPawn && PowerUpTrigger && CurrentPowerUp) // Pacman - PowerUp overlap.
	{
		check(PowerUpTrigger == CurrentPowerUp);
		const int32 DiceRoll = FMath::RandRange(1, 100);
		if (DiceRoll <= 5)
		{
			GPacmanNumLives += 1;
			HUDWidget->LivesText->SetText(FText::Format(LOCTEXT("Lives", "Lives: {0}"), GPacmanNumLives));
		}
		else if (DiceRoll <= 15)
		{
			GPacmanNumRandomTeleports += 1;
			HUDWidget->RandomTeleportsText->SetText(FText::Format(LOCTEXT("RandomTeleports", "Teleports: {0}"), GPacmanNumRandomTeleports));
		}
		else
		{
			GPacmanScore += 200;
			HUDWidget->ScoreText->SetText(FText::Format(LOCTEXT("Score", "Score: {0}"), GPacmanScore));
		}
		PowerUpTrigger->Destroy();
		CurrentPowerUp = nullptr;
		PowerUpTimer = GPowerUpSpawnPeriod;
	}
	else if (GhostPawn) // Ghost - Ghost overlap.
	{
		AGhostPawn* SecondGhostPawn = Cast<AGhostPawn>(PacmanOrGhost);
		if (SecondGhostPawn && GhostPawn->FrozenModeTimer != 0.75f)
		{
			SecondGhostPawn->FrozenModeTimer = 0.75f;
		}
	}
}

void APacmanGameModeBase::CompleteLevel()
{
	GenericInfoWidget->Text->SetText(FText::Format(LOCTEXT("CompleteLevel", "You have completed Level {0}. Congratulations!"), GGameLevel));
	GenericInfoWidget->AddToViewport();
	GetWorldTimerManager().SetTimer(TimerHandle,
		[this]()
		{
			GenericInfoWidget->RemoveFromViewport();
			UGameplayStatics::OpenLevel(GetWorld(), TEXT("Level_01"));
		},
		2.0f, false);
}

void APacmanGameModeBase::BeginFrightenedMode()
{
	FrightenedModeTimer = GFrightenedModeDuration;
	DirectionUpdateTimer = 10000.0f; // Ghosts will change direction immediately.

	for (AGhostPawn* Ghost : Ghosts)
	{
		if (!Ghost->bIsInHouse)
		{
			Ghost->bIsFrightened = true;
			FTeleport& Teleport = Teleports[(int32)Ghost->Color];
			if (Teleport.Material != nullptr)
			{
				Ghost->Material->SetScalarParameterValue(TEXT("Opacity"), GGhostDefaultOpacity);
				Teleport.Material = Ghost->FrightenedMaterial;
				Teleport.Material->SetScalarParameterValue(TEXT("Opacity"), Teleport.Opacity);
			}
			Ghost->SetFrightenedMaterial();
		}
	}
}

void APacmanGameModeBase::ShowGetReadyInfoWidget()
{
	GenericInfoWidget->Text->SetText(LOCTEXT("Ready", "Get Ready!"));
	GenericInfoWidget->AddToViewport();
	GetWorldTimerManager().SetTimer(TimerHandle,
		[this]()
		{
			GenericInfoWidget->RemoveFromViewport();
		},
		2.0f, false);
}

void APacmanGameModeBase::DoRandomTeleport()
{
	if (GPacmanNumRandomTeleports == 0 || GGameLevel == 0 || GenericInfoWidget->IsInViewport() || Teleports[(int32)EGhostColor::Orange + 1].Material)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	GPacmanNumRandomTeleports -= 1;
	HUDWidget->RandomTeleportsText->SetText(FText::Format(LOCTEXT("RandomTeleports", "Teleports: {0}"), GPacmanNumRandomTeleports));

	const FVector RandomLocation = SelectRandomLocationOnMap(World, Pacman->GetActorLocation());
	Pacman->SelectRandomDirection(RandomLocation);
	Pacman->Material->SetScalarParameterValue(TEXT("Opacity"), 1.0f);
	Teleports[(int32)EGhostColor::Orange + 1] =
	{
		Pacman->Material, 1.0f, -1.0f,
		[this, RandomLocation]()
		{
			Pacman->SetActorLocation(RandomLocation, false, nullptr, ETeleportType::ResetPhysics);
		},
	};
}

FVector APacmanGameModeBase::SelectRandomLocationOnMap(UWorld* World, const FVector& CurrentLocation)
{
	check(World != nullptr);

	FVector RandomLocation = {};
	for (;;)
	{
		RandomLocation.X = FMath::RandRange(0, GMapTileNumX - 1) * GMapTileSize - ((GMapTileSize * GMapTileNumX) / 2) - GMapTileSize / 2;
		RandomLocation.Y = FMath::RandRange(0, GMapTileNumY - 1) * GMapTileSize - ((GMapTileSize * GMapTileNumY) / 2) - GMapTileSize / 2;
		RandomLocation.Z = GMapTileSize / 2;

		bool bIsBlocked = World->SweepTestByChannel(
			RandomLocation + FVector(0.0f, 0.0f, GMapTileSize * 2),
			RandomLocation + FVector(0.0f, 0.0f, GMapTileSize / 2 + 10.0f),
			FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(GMapTileSize / 2 - 1.0f));

		for (AGhostPawn* Ghost : Ghosts)
		{
			if ((Ghost->HouseLocation - RandomLocation).IsNearlyZero(1.5f))
			{
				bIsBlocked = true;
				break;
			}
		}
		if ((Ghosts[(int32)EGhostColor::Red]->HouseLocation - (RandomLocation + FVector(-GMapTileSize, 0.0f, 0.0f))).IsNearlyZero(1.5f))
		{
			bIsBlocked = true;
		}
		if ((Ghosts[(int32)EGhostColor::Red]->HouseLocation - (RandomLocation + FVector(GMapTileSize, 0.0f, 0.0f))).IsNearlyZero(1.5f))
		{
			bIsBlocked = true;
		}
		if (FVector::Distance(RandomLocation, CurrentLocation.GridSnap(GMapTileSize / 2)) < 300.0f)
		{
			bIsBlocked = true;
		}
		if (!bIsBlocked)
		{
			break;
		}
	}
	return RandomLocation;
}


#include "Modules/ModuleManager.h"
IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Pacman, "Pacman");

#undef LOCTEXT_NAMESPACE
