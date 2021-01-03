#pragma once
#include "GameFramework/Actor.h"
#include "PacmanFood.generated.h"

class UStaticMeshComponent;

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
