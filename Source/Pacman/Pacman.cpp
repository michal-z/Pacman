#include "Pacman.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, Pacman, "Pacman");

AActor* FindActorByName(const FString& Name, UWorld* World)
{
	for (ULevel* Level : World->GetLevels())
	{
		if (Level && Level->IsCurrentLevel())
		{
			for (AActor* Actor : Level->Actors)
			{
				if (Actor && Actor->GetName() == TEXT("MainCamera"))
				{
					return Actor;
				}
			}
		}
	}
	return nullptr;
}
