#pragma once
#include "Blueprint/UserWidget.h"
#include "PacmanHUDWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS(Abstract)
class PACMAN_API UPacmanHUDWidget : public UUserWidget
{
	public: UPROPERTY(meta = (BindWidget)) UTextBlock* ScoreText;
	public: UPROPERTY(meta = (BindWidget)) UTextBlock* LivesText;

	GENERATED_BODY()
};
