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
	UMaterial* TeleportBaseMaterial;
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
	FTimerHandle TimerHandle;
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	TSubclassOf<UUserWidget> GenericInfoWidgetClass;
	struct
	{
		UMaterialInstanceDynamic* Material;
		float Opacity;
		float Sign;
		TFunction<void()> CalledWhenOpacity0;
		TFunction<void()> CalledWhenOpacity1;
	}
	Teleport;
	UPROPERTY() UUserWidget* MainMenuWidget;
	UPROPERTY() UUserWidget* PauseMenuWidget;
	UPROPERTY() UGenericInfoWidget* GenericInfoWidget;


	APacmanGameModeBase();
	void PauseGame();
	void ResumeGame();
	void BeginNewGame();
	void ReturnToMainMenu();
	void QuitGame();
	void NotifyGhostBeginOverlap(AActor* PacmanOrGhost, AGhostPawn* InGhost);
	void CompleteLevel();
	void BeginFrightenedMode();
	void MoveGhosts(float DeltaTime);
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void ShowGetReadyInfoWidget();

	GENERATED_BODY()
};
