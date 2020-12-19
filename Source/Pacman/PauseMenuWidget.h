#pragma once

#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;

UCLASS()
class PACMAN_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UButton* ResumeGameButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ReturnToMainMenuButton;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnResumeGame();

	UFUNCTION()
	void OnReturnToMainMenu();
};
