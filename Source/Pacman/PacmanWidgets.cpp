#include "PacmanWidgets.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"

void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NewGameButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnNewGame);
	QuitGameButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnQuitGame);
}

void UMainMenuWidget::OnNewGame()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->BeginNewGame();
	RemoveFromParent();
}

void UMainMenuWidget::OnQuitGame()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->QuitGame();
	RemoveFromParent();
}


void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResumeGameButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnResumeGame);
	ReturnToMainMenuButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnReturnToMainMenu);
}

void UPauseMenuWidget::OnResumeGame()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->ResumeGame();
	RemoveFromParent();
}

void UPauseMenuWidget::OnReturnToMainMenu()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->ReturnToMainMenu();
	RemoveFromParent();
}