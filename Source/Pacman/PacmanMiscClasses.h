#pragma once
#include "GameFramework/Actor.h"
#include "GameFramework/SaveGame.h"
#include "Blueprint/UserWidget.h"
#include "PacmanMiscClasses.generated.h"

class UTextBlock;
class UButton;
class UStaticMeshComponent;

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

USTRUCT()
struct FHiscoreEntry
{
	UPROPERTY()
	FString Name;

	UPROPERTY()
	uint32 Score;

	GENERATED_BODY()
};

inline bool operator<(const FHiscoreEntry& Lhs, const FHiscoreEntry& Rhs)
{
	return Lhs.Score < Rhs.Score;
}

UCLASS()
class PACMAN_API UPacmanHiscore : public USaveGame
{
public:
	UPROPERTY()
	TArray<FHiscoreEntry> Entries;

	GENERATED_BODY()
};

UCLASS()
class PACMAN_API APacmanFood : public AActor
{
public:
	APacmanFood();
	uint32 GetScore() const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Pacman Food")
	uint32 Score;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	GENERATED_BODY()
};

FORCEINLINE uint32 APacmanFood::GetScore() const
{
	return Score;
}
