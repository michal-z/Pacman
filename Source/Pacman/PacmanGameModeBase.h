#pragma once

#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

class UGenericInfoWidget;
class APacmanPawn;
class AGhostPawn;

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

private:
	bool bIsInGameLevel;
	bool bShowInfoWidget;
	APacmanPawn* Pacman;
	TArray<AGhostPawn*> Ghosts;
	float DirectionUpdateTime;

	void MovePacman(float DeltaTime);
	void MoveGhosts(float DeltaTime);

	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	TSubclassOf<UUserWidget> GenericInfoWidgetClass;

	UPROPERTY() UUserWidget* MainMenuWidget;
	UPROPERTY() UUserWidget* PauseMenuWidget;
	UPROPERTY() UGenericInfoWidget* GenericInfoWidget;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void ShowGetReadyInfoWidget();
	FTimerHandle Timer;

	GENERATED_BODY()
};
