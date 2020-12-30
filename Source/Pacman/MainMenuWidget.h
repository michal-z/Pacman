#pragma once
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

UCLASS(Abstract)
class PACMAN_API UMainMenuWidget : public UUserWidget
{
	UPROPERTY(meta = (BindWidget))
	UButton* NewGameButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitGameButton;

	virtual void NativeConstruct() override;

	UFUNCTION() void OnNewGame();
	UFUNCTION() void OnQuitGame();

	GENERATED_BODY()
};
