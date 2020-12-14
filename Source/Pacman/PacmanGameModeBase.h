#pragma once

#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

UCLASS()
class PACMAN_API APacmanGameModeBase : public AGameModeBase
{
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;

	UPROPERTY()
	UUserWidget* MainMenuWidget = nullptr;

	UPROPERTY()
	UUserWidget* PauseMenuWidget = nullptr;

	virtual void BeginPlay() override;

public:
	void PauseGame();
	void ResumeGame();
	void BeginNewGame();
	void ReturnToMainMenu();
	void QuitGame();

	GENERATED_BODY()
};
