#include "FayeGameMode.h"
#include "FayePlayerController.h"
#include "FayeCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Boids/BoidGroupsController.h"
#include "ScoreManager.h"

AFayeGameMode::AFayeGameMode()
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.bCanEverTick = false;

	m_difficultyManager = CreateDefaultSubobject< UDifficultyManager >( "DifficultyManager" );
	m_boidGroupsController = CreateDefaultSubobject< UBoidGroupsController >( "BoidGroupsController" );
	m_scoreManager = CreateDefaultSubobject< UScoreManager >( "ScoreManager" );
}

UBoidGroupsController* AFayeGameMode::GetBoidGroupsController()
{
	return m_boidGroupsController;
}

UDifficultyManager* AFayeGameMode::GetDifficultyManager()
{
	return m_difficultyManager;
}

UScoreManager* AFayeGameMode::GetScoreManager()
{
	return m_scoreManager;
}