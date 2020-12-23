#pragma once

#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

class UGenericInfoWidget;

UCLASS()
class PACMAN_API APacmanGameModeBase : public AGameModeBase
{
public:
	APacmanGameModeBase();

	void PauseGame();
	void ResumeGame();
	void BeginNewGame();
	void ReturnToMainMenu();
	void QuitGame();
	void KillPacman();

	bool GetIsReady() const { return bIsReady; }

private:
	bool bIsReady;

	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	TSubclassOf<UUserWidget> GenericInfoWidgetClass;

	UPROPERTY()
	UUserWidget* MainMenuWidget;

	UPROPERTY()
	UUserWidget* PauseMenuWidget;

	UPROPERTY()
	UGenericInfoWidget* GenericInfoWidget;

	virtual void BeginPlay() override;

	void OpenGenericInfoWidget();
	void CloseGenericInfoWidget();
	FTimerHandle Timer;

	GENERATED_BODY()
};
