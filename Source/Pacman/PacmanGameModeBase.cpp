#include "PacmanGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "PacmanPawn.h"
#include "GhostPawn.h"
#include "GenericInfoWidget.h"

PRAGMA_DISABLE_OPTIMIZATION

#define LOCTEXT_NAMESPACE "PacmanGameModeBase"

APacmanGameModeBase::APacmanGameModeBase()
{
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

	bIsReady = false;
}

void APacmanGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	check(GEngine);

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

		OpenGenericInfoWidget();
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
	APacmanPawn* Pacman = CastChecked<APacmanPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass()));

	if (Pacman->Kill() == 0)
	{
		ReturnToMainMenu();
	}
	else
	{
		TArray<AActor*> Ghosts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostPawn::StaticClass(), Ghosts);
		check(Ghosts.Num() > 0);

		for (AActor* Actor : Ghosts)
		{
			AGhostPawn* Ghost = CastChecked<AGhostPawn>(Actor);
			Ghost->SetInitialState();
		}

		OpenGenericInfoWidget();
	}
}

void APacmanGameModeBase::OpenGenericInfoWidget()
{
	UWorld* World = GetWorld();
	if (World)
	{
		GenericInfoWidget->Text->SetText(LOCTEXT("Ready", "Get Ready!"));
		GenericInfoWidget->AddToViewport();

		bIsReady = false;

		World->GetTimerManager().SetTimer(Timer, this, &APacmanGameModeBase::CloseGenericInfoWidget, 2.0f);
	}
}

void APacmanGameModeBase::CloseGenericInfoWidget()
{
	GenericInfoWidget->RemoveFromViewport();

	bIsReady = true;
}

#undef LOCTEXT_NAMESPACE
