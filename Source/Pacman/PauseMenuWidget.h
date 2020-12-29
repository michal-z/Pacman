#pragma once
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

UCLASS(Abstract)
class PACMAN_API UPauseMenuWidget : public UUserWidget
{
	private: UPROPERTY(meta = (BindWidget))
			 UButton* ResumeGameButton;

	private: UPROPERTY(meta = (BindWidget))
			 UButton* ReturnToMainMenuButton;

	private: virtual void NativeConstruct() override;

	private: UFUNCTION() void OnResumeGame();
	private: UFUNCTION() void OnReturnToMainMenu();

	GENERATED_BODY()
};
