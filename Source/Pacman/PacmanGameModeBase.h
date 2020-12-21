#pragma once

#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

UCLASS()
class PACMAN_API APacmanGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY()
	UUserWidget* MainMenuWidget;

	UPROPERTY()
	UUserWidget* PauseMenuWidget;

	virtual void BeginPlay() override;

public:
	APacmanGameModeBase();

	void PauseGame();
	void ResumeGame();
	void BeginNewGame();
	void ReturnToMainMenu();
	void QuitGame();
	void KillPacman();
};
