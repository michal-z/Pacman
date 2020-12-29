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
	void NotifyGhostBeginOverlap(AActor* PacmanOrGhost, AGhostPawn* InGhost);
	void CompleteLevel();
	void BeginFrightenedMode();

private:
	bool bShowInfoWidget;
	uint32 GameLevel;
	APacmanPawn* Pacman;
	TArray<AGhostPawn*> Ghosts;
	UMaterialInstance* GhostFrightenedModeMaterial;
	float FrightenedModeTimer;
	float DirectionUpdateTimer;
	// Odd GhostModeIndex is Scatter mode. Even GhostModeIndex is Chase mode.
	static constexpr float GhostModeDurations[] = { 7.0f, 20.0f, 5.0f, 20.0f, 3.0f };
	float GhostModeTimer;
	uint32 GhostModeIndex;

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
	FTimerHandle TimerHandle;

	GENERATED_BODY()
};
