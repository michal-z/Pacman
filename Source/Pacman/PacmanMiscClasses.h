#pragma once
#include "Blueprint/UserWidget.h"
#include "GameFramework/SaveGame.h"
#include "PacmanMiscClasses.generated.h"

class UTextBlock;
class UButton;

UCLASS(Abstract)
class PACMAN_API UGenericInfoWidget : public UUserWidget
{
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text;

	GENERATED_BODY()
};

UCLASS(Abstract)
class PACMAN_API UMainMenuWidget : public UUserWidget
{
private:
	UPROPERTY(meta = (BindWidget)) UButton* NewGameButton;
	UPROPERTY(meta = (BindWidget)) UButton* QuitGameButton;

	virtual void NativeConstruct() override;

	UFUNCTION() void OnNewGame();
	UFUNCTION() void OnQuitGame();

	GENERATED_BODY()
};

UCLASS(Abstract)
class PACMAN_API UPauseMenuWidget : public UUserWidget
{
private:
	UPROPERTY(meta = (BindWidget)) UButton* ResumeGameButton;
	UPROPERTY(meta = (BindWidget)) UButton* ReturnToMainMenuButton;

	virtual void NativeConstruct() override;

	UFUNCTION() void OnResumeGame();
	UFUNCTION() void OnReturnToMainMenu();

	GENERATED_BODY()
};

UCLASS(Abstract)
class PACMAN_API UPacmanHUDWidget : public UUserWidget
{
public:
	UPROPERTY(meta = (BindWidget)) UTextBlock* ScoreText;
	UPROPERTY(meta = (BindWidget)) UTextBlock* LivesText;

	GENERATED_BODY()
};

UCLASS()
class PACMAN_API UPacmanHighscore : public USaveGame
{
public:
	UPROPERTY()
	TArray<int32> Scores;

	GENERATED_BODY()
};
