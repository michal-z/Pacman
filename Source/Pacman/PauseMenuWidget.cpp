#include "PauseMenuWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"

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