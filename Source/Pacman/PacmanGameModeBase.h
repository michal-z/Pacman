#pragma once

#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

UCLASS()
class PACMAN_API APacmanGameModeBase : public AGameModeBase
{
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY()
	UUserWidget* MainMenuWidget = nullptr;

	UPROPERTY()
	UUserWidget* PauseMenuWidget = nullptr;

	virtual void BeginPlay() override;

public:
	APacmanGameModeBase();

	void PauseGame();
	void ResumeGame();
	void BeginNewGame();
	void ReturnToMainMenu();
	void QuitGame();

	GENERATED_BODY()
};
