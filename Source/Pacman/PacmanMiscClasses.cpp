#include "PacmanMiscClasses.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"

void UGenericInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	PlayerName->OnTextCommitted.AddUniqueDynamic(GameMode, &APacmanGameModeBase::OnPlayerNameCommit);
}


void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NewGameButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnNewGame);
	QuitGameButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnQuitGame);
}

void UMainMenuWidget::OnNewGame()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->BeginNewGame();
	RemoveFromParent();
}

void UMainMenuWidget::OnQuitGame()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->QuitGame();
	RemoveFromParent();
}


void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResumeGameButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnResumeGame);
	ReturnToMainMenuButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnReturnToMainMenu);
}

void UPauseMenuWidget::OnResumeGame()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->ResumeGame();
	RemoveFromParent();
}

void UPauseMenuWidget::OnReturnToMainMenu()
{
	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	GameMode->ReturnToMainMenu();
	RemoveFromParent();
}


APacmanFood::APacmanFood()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	MeshComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	MeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	RootComponent = MeshComponent;

	Score = 1;
}
