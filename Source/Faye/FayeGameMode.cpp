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

TWeakObjectPtr< UBoidGroupsController > AFayeGameMode::GetBoidGroupsController()
{
	return m_boidGroupsController;
}

TWeakObjectPtr< UDifficultyManager > AFayeGameMode::GetDifficultyManager()
{
	return m_difficultyManager;
}

TWeakObjectPtr< UScoreManager > AFayeGameMode::GetScoreManager()
{
	return m_scoreManager;
}