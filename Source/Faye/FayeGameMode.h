#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DifficultyManager.h"
#include "FayeGameMode.generated.h"

class UBoidGroupsController;
class UDifficultyManager;
class UScoreManager;

UCLASS(minimalapi)
class AFayeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AFayeGameMode();

	UBoidGroupsController* GetBoidGroupsController();
	UDifficultyManager* GetDifficultyManager();
	UScoreManager* GetScoreManager();

private:
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, DisplayName = "Difficulty Manager", meta = (AllowPrivateAccess = "true") )
	UDifficultyManager* m_difficultyManager;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, DisplayName = "Boid Groups Controller", meta = (AllowPrivateAccess = "true") )
	UBoidGroupsController* m_boidGroupsController;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, DisplayName = "Score Manager", meta = (AllowPrivateAccess = "true") )
	UScoreManager* m_scoreManager;
};



