#include "PacmanMiscClasses.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"

void UGenericInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	PlayerName->OnTextCommitted.AddUniqueDynamic(GameMode, &APacmanGameModeBase::SaveHiscoreName);
}


void UMainMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	NewGameButton->OnClicked.AddUniqueDynamic(GameMode, &APacmanGameModeBase::BeginNewGame);
	QuitGameButton->OnClicked.AddUniqueDynamic(GameMode, &APacmanGameModeBase::QuitGame);
}


void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	ResumeGameButton->OnClicked.AddUniqueDynamic(GameMode, &APacmanGameModeBase::ResumeGame);
	ReturnToMainMenuButton->OnClicked.AddUniqueDynamic(GameMode, &APacmanGameModeBase::ReturnToMainMenu);
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
