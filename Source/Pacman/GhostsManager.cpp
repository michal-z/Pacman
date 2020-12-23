#include "GhostsManager.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "PacmanGameModeBase.h"
#include "PacmanPawn.h"
#include "GhostPawn.h"

AGhostsManager::AGhostsManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGhostsManager::BeginPlay()
{
	Super::BeginPlay();

	PacmanGameMode = CastChecked<APacmanGameModeBase>(UGameplayStatics::GetGameMode(GetWorld()));

	Pacman = CastChecked<APacmanPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), APacmanPawn::StaticClass()));

	TArray<AActor*> GhostActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGhostPawn::StaticClass(), GhostActors);

	for (AActor* Actor : GhostActors)
	{
		Ghosts.Add(CastChecked<AGhostPawn>(Actor));
	}

	DirectionUpdateTime = 0.0f;
}

void AGhostsManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PacmanGameMode->GetIsReady() == false)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (World == nullptr)
	{
		return;
	}

	if ((DirectionUpdateTime += DeltaTime) > 0.25f)
	{
		DirectionUpdateTime = 0.0f;

		for (AGhostPawn* Ghost : Ghosts)
		{
			const float Radius = Ghost->CollisionComponent->GetScaledSphereRadius();
			const FVector GhostLocation = Ghost->GetActorLocation();
			const FVector GhostDirection = Ghost->CurrentDirection;

			FVector TargetLocation;
			switch (Ghost->GhostColor)
			{
			case EGhostColor::Red:
				TargetLocation = Pacman->GetActorLocation();
				//TargetLocation = FVector(950.0f, -650.0f, 50.0f);
				break;
			case EGhostColor::Pink:
				TargetLocation = Pacman->GetActorLocation() + 4 * 100.0f * Pacman->CurrentDirection;
				//TargetLocation = FVector(-750.0f, -750.0f, 50.0f);
				break;
			case EGhostColor::Blue:
				TargetLocation = Ghosts[0]->GetActorLocation() + 2.0f * ((Pacman->GetActorLocation() + 2 * 100.0f * Pacman->CurrentDirection) - Ghosts[0]->GetActorLocation());
				//TargetLocation = FVector(450.0f, 950.0f, 50.0f);
				break;
			case EGhostColor::Orange: {
				const float Distance = FVector::Distance(Pacman->GetActorLocation(), GhostLocation);
				if (Distance >= 8.0f * 100.0f)
				{
					TargetLocation = Pacman->GetActorLocation();
				}
				else
				{
					TargetLocation = FVector(-950.0f, 750.0f, 50.0f);
				}
				break;
			}
			default:
				check(false);
			}

			const float Speed = DeltaTime * Ghost->Speed;
			const FVector Destinations[3] =
			{
				GhostLocation + GhostDirection * Speed,
				GhostLocation + FVector(GhostDirection.Y, GhostDirection.X, 0.0f) * Speed,
				GhostLocation + FVector(-GhostDirection.Y, -GhostDirection.X, 0.0f) * Speed,
			};
			float Distances[3] = {};
			bool bIsBlocked[3] = {};

			for (uint32 Idx = 0; Idx < 3; ++Idx)
			{
				bIsBlocked[Idx] = World->SweepTestByChannel(GhostLocation, Destinations[Idx], FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Radius));

				Distances[Idx] = FVector::Distance(Destinations[Idx], TargetLocation);
			}

			int32 SelectedDirection = -1;
			float MinDistance = 100000.0f;
			for (uint32 Idx = 0; Idx < 3; ++Idx)
			{
				if (bIsBlocked[Idx] == false && Distances[Idx] < MinDistance)
				{
					MinDistance = Distances[Idx];
					SelectedDirection = (int32)Idx;
				}
			}

			FVector WantedDirection;

			if (SelectedDirection == 0)
			{
				WantedDirection = GhostDirection;
			}
			else if (SelectedDirection == 1)
			{
				WantedDirection = FVector(GhostDirection.Y, GhostDirection.X, 0.0f);
			}
			else if (SelectedDirection == 2)
			{
				WantedDirection = FVector(-GhostDirection.Y, -GhostDirection.X, 0.0f);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("GhostsManager: All paths blocked!"));
				WantedDirection = GhostDirection;
				DirectionUpdateTime = 1.0f;
			}

			Ghost->CurrentDirection = WantedDirection;
		}
	}

	for (AGhostPawn* Ghost : Ghosts)
	{
		const FVector Delta = Ghost->CurrentDirection * Ghost->Speed * DeltaTime;

		FHitResult Hit;
		Ghost->MovementComponent->SafeMoveUpdatedComponent(Delta, FQuat::Identity, true, Hit);

		if (Hit.IsValidBlockingHit())
		{
			Ghost->MovementComponent->SlideAlongSurface(Delta, 1.0f - Hit.Time, Hit.Normal, Hit);
		}
	}
}
