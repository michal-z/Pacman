#pragma once
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;

UCLASS(Abstract)
class PACMAN_API UMainMenuWidget : public UUserWidget
{
	private: UPROPERTY(meta = (BindWidget)) UButton* NewGameButton;
	private: UPROPERTY(meta = (BindWidget)) UButton* QuitGameButton;

	private: virtual void NativeConstruct() override;

	private: UFUNCTION() void OnNewGame();
	private: UFUNCTION() void OnQuitGame();

	GENERATED_BODY()
};
