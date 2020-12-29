#pragma once

#include "Blueprint/UserWidget.h"
#include "GenericInfoWidget.generated.h"

class UTextBlock;

UCLASS(Abstract)
class PACMAN_API UGenericInfoWidget : public UUserWidget
{
public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* Text;

	GENERATED_BODY()
};
