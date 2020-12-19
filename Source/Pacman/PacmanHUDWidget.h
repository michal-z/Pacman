#pragma once

#include "Blueprint/UserWidget.h"
#include "PacmanHUDWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS(Abstract)
class PACMAN_API UPacmanHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreText;
};
