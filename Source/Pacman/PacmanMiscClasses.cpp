#include "PacmanMiscClasses.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/SphereComponent.h"
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

	NewGameButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnNewGame);
	QuitGameButton->OnClicked.AddUniqueDynamic(this, &UMainMenuWidget::OnQuitGame);
}

void UMainMenuWidget::OnNewGame()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Level_01"));
}

void UMainMenuWidget::OnQuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, true);
}


void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	APacmanGameModeBase* GameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	ResumeGameButton->OnClicked.AddUniqueDynamic(GameMode, &APacmanGameModeBase::ResumeGame);

	ReturnToMainMenuButton->OnClicked.AddUniqueDynamic(this, &UPauseMenuWidget::OnReturnToMainMenu);
}

void UPauseMenuWidget::OnReturnToMainMenu()
{
	UGameplayStatics::OpenLevel(GetWorld(), TEXT("Main"));
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


APowerUpTrigger::APowerUpTrigger()
{
	//SetHidden(true);
	//SetCanBeDamaged(false);

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComponent->InitSphereRadius((GMapTileSize / 2) * 0.4f);
	//CollisionComponent->bHiddenInGame = false;

	UStaticMeshComponent* VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualComponent"));
	VisualComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualComponent->SetupAttachment(CollisionComponent);

	RootComponent = CollisionComponent;

	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> Finder(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		VisualComponent->SetStaticMesh(Finder.Object);
		VisualComponent->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f));
	}
}

void APowerUpTrigger::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	APacmanGameModeBase* GameMode = Cast<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GameMode && OtherActor)
	{
		GameMode->HandleActorOverlap(OtherActor, this);
	}
}
