#pragma once
#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

class UGenericInfoWidget;
class APacmanPawn;
class AGhostPawn;

UCLASS() class PACMAN_API APacmanGameModeBase : public AGameModeBase
{
	private: bool bShowInfoWidget;
	private: uint32 GameLevel;
	private: APacmanPawn* Pacman;
	private: TArray<AGhostPawn*> Ghosts;
	private: UMaterialInstance* GhostFrightenedModeMaterial;
	private: float FrightenedModeTimer;
	private: float DirectionUpdateTimer;
	// Odd GhostModeIndex is Scatter mode. Even GhostModeIndex is Chase mode.
	private: static constexpr float GhostModeDurations[] = { 7.0f, 20.0f, 5.0f, 20.0f, 3.0f };
	private: float GhostModeTimer;
	private: uint32 GhostModeIndex;
	private: FTimerHandle TimerHandle;
	private: TSubclassOf<UUserWidget> MainMenuWidgetClass;
	private: TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	private: TSubclassOf<UUserWidget> GenericInfoWidgetClass;

	private: UPROPERTY() UUserWidget* MainMenuWidget;
	private: UPROPERTY() UUserWidget* PauseMenuWidget;
	private: UPROPERTY() UGenericInfoWidget* GenericInfoWidget;


	public: APacmanGameModeBase();
	public: void PauseGame();
	public: void ResumeGame();
	public: void BeginNewGame();
	public: void ReturnToMainMenu();
	public: void QuitGame();
	public: void NotifyGhostBeginOverlap(AActor* PacmanOrGhost, AGhostPawn* InGhost);
	public: void CompleteLevel();
	public: void BeginFrightenedMode();
	private: void MoveGhosts(float DeltaTime);
	private: virtual void BeginPlay() override;
	private: virtual void Tick(float DeltaTime) override;
	private: void ShowGetReadyInfoWidget();

	GENERATED_BODY()
};
