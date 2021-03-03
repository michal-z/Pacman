#pragma once
#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

constexpr float GMapTileSize = 100.0f;
constexpr float GGhostDefaultOpacity = 0.9f;

class UGenericInfoWidget;
class UMainMenuWidget;
class UPacmanHUDWidget;
class UNiagaraSystem;
class APacmanPawn;
class APacmanFood;
class AGhostPawn;
class APowerUpTrigger;

UCLASS()
class PACMAN_API APacmanGameModeBase : public AGameModeBase
{
public:
	APacmanGameModeBase();
	UFUNCTION() void SaveHiscoreName(const FText& InText, ETextCommit::Type CommitMethod);
	UFUNCTION() void NewGame();
	UFUNCTION() void QuitGame();
	UFUNCTION() void ReturnToMainMenu();
	UFUNCTION() void ResumeGame();
	void PauseGame();
	void DoRandomTeleport();
	void HandleActorOverlap(AActor* PacmanOrGhost, AActor* Other);

	UPROPERTY(EditAnywhere) UMaterialInterface* GhostFrightenedMaterial;

private:
	UPROPERTY() UNiagaraSystem* SuperFoodFX;
	UPROPERTY() UNiagaraSystem* PowerUpFX;
	UPROPERTY() APacmanPawn* Pacman;
	TArray<AGhostPawn*> Ghosts;
	UPROPERTY() AActor* CurrentPowerUp;
	FVector CurrentPowerUpLocation;
	float PowerUpTimer;
	float FrightenedModeTimer;
	float DirectionUpdateTimer;
	// Even GhostModeIndex is Scatter mode. Odd GhostModeIndex is Chase mode.
	static constexpr float GhostModeDurations[] = { 15.0f, 30.0f, 15.0f, 30.0f, 10.0f };
	float GhostModeTimer;
	uint32 GhostModeIndex;
	uint32 NumFoodLeft;
	FTimerHandle TimerHandle;
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	TSubclassOf<UUserWidget> GenericInfoWidgetClass;
	TSubclassOf<UUserWidget> HUDWidgetClass;
	TSubclassOf<APacmanFood> SuperFoodClass;
	TSubclassOf<APowerUpTrigger> PowerUpTriggerClass;
	UPROPERTY() UMainMenuWidget* MainMenuWidget;
	UPROPERTY() UUserWidget* PauseMenuWidget;
	UPROPERTY() UGenericInfoWidget* GenericInfoWidget;
	UPROPERTY() UPacmanHUDWidget* HUDWidget;
	struct FTeleport
	{
		UMaterialInstanceDynamic* Material;
		float Opacity;
		float Sign;
		TFunction<void()> CalledWhenOpacity0;
		//TFunction<void()> CalledWhenOpacity1;
	};
	FTeleport Teleports[5];

	void MoveGhosts(float DeltaTime);
	void ShowGetReadyInfoWidget();
	uint32 KillPacman();
	void CompleteLevel();
	void BeginFrightenedMode();
	FVector SelectRandomLocationOnMap(UWorld* World, const FVector& CurrentLocation);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	GENERATED_BODY()
};
