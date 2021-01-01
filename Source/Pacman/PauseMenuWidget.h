#pragma once
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

UCLASS(Abstract)
class PACMAN_API UPauseMenuWidget : public UUserWidget
{
	UPROPERTY(meta = (BindWidget)) UButton* ResumeGameButton;
	UPROPERTY(meta = (BindWidget)) UButton* ReturnToMainMenuButton;

	virtual void NativeConstruct() override;

	UFUNCTION() void OnResumeGame();
	UFUNCTION() void OnReturnToMainMenu();

	GENERATED_BODY()
};
