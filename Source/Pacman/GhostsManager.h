#pragma once

#include "GameFramework/Actor.h"
#include "GhostsManager.generated.h"

class APacmanPawn;
class AGhostPawn;

UCLASS()
class PACMAN_API AGhostsManager : public AActor
{
public:
	AGhostsManager();

private:
	APacmanPawn* Pacman;
	TArray<AGhostPawn*> Ghosts;
	float DirectionUpdateTime;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	GENERATED_BODY()
};
