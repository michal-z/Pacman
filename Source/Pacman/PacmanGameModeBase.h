#pragma once
#include "GameFramework/GameModeBase.h"
#include "PacmanGameModeBase.generated.h"

class UGenericInfoWidget;
class UMainMenuWidget;
class UPacmanHUDWidget;
class APacmanPawn;
class APacmanFood;
class AGhostPawn;

UCLASS()
class PACMAN_API APacmanGameModeBase : public AGameModeBase
{
public:
	APacmanGameModeBase();
	UFUNCTION() void SaveHiscoreName(const FText& InText, ETextCommit::Type CommitMethod);
	UFUNCTION() void ResumeGame();
	void PauseGame();
	void HandleActorOverlap(AActor* PacmanOrGhost, AActor* Other);
	UMaterial* GetTeleportBaseMaterial() const;
	UMaterialInstance* GetGhostFrightenedModeMaterial() const;

private:
	UMaterial* TeleportBaseMaterial;
	APacmanPawn* Pacman;
	TArray<AGhostPawn*> Ghosts;
	UMaterialInstance* GhostFrightenedModeMaterial;
	float FrightenedModeTimer;
	float DirectionUpdateTimer;
	// Odd GhostModeIndex is Scatter mode. Even GhostModeIndex is Chase mode.
	static constexpr float GhostModeDurations[] = { 7.0f, 20.0f, 5.0f, 20.0f, 3.0f };
	float GhostModeTimer;
	uint32 GhostModeIndex;
	uint32 NumFoodLeft;
	FTimerHandle TimerHandle;
	TSubclassOf<UUserWidget> MainMenuWidgetClass;
	TSubclassOf<UUserWidget> PauseMenuWidgetClass;
	TSubclassOf<UUserWidget> GenericInfoWidgetClass;
	TSubclassOf<UUserWidget> HUDWidgetClass;
	TSubclassOf<APacmanFood> SuperFoodClass;
	UPROPERTY() UMainMenuWidget* MainMenuWidget;
	UPROPERTY() UUserWidget* PauseMenuWidget;
	UPROPERTY() UGenericInfoWidget* GenericInfoWidget;
	UPROPERTY() UPacmanHUDWidget* HUDWidget;
	struct
	{
		UMaterialInstanceDynamic* Material;
		float Opacity;
		float Sign;
		TFunction<void()> CalledWhenOpacity0;
		TFunction<void()> CalledWhenOpacity1;
	} Teleport;

	void MoveGhosts(float DeltaTime);
	void ShowGetReadyInfoWidget();
	uint32 KillPacman();
	void CompleteLevel();
	void BeginFrightenedMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	GENERATED_BODY()
};

FORCEINLINE UMaterial* APacmanGameModeBase::GetTeleportBaseMaterial() const
{
	return TeleportBaseMaterial;
}

FORCEINLINE UMaterialInstance* APacmanGameModeBase::GetGhostFrightenedModeMaterial() const
{
	return GhostFrightenedModeMaterial;
}
