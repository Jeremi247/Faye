#include "ScoreManager.h"
#include "DifficultyManager.h"
#include "FayeGameMode.h"

UScoreManager::UScoreManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickInterval = 1.f;
}

void UScoreManager::BeginPlay()
{
	Super::BeginPlay();

	AFayeGameMode* gameMode = Cast< AFayeGameMode >( GetWorld()->GetAuthGameMode() );
	m_difficultyManager = gameMode->GetDifficultyManager();
}

void UScoreManager::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	ValidateScorePerMinute();
	ensureAlways( m_difficultyManager.IsValid() );
	m_difficultyManager->VerifyDifficultyLevel( m_pointsPerMinute );
}

void UScoreManager::AddScore( int score )
{
	float scoreTime = GetWorld()->GetTimeSeconds();

	m_scoreTimeStamps.Emplace( scoreTime, score );
	m_globalScore += score;
}

void UScoreManager::ValidateScorePerMinute()
{
	m_pointsPerMinute = 0.f;

	float currentTime = GetWorld()->GetTimeSeconds();
	while( m_scoreTimeStamps.Num() > 0 && m_scoreTimeStamps[0].first < currentTime - 60.f )
	{
		m_scoreTimeStamps.RemoveAt( 0 );
	}

	for( const std::pair< float, float >& pair : m_scoreTimeStamps )
	{
		m_pointsPerMinute += pair.second;
	}

	if( m_pointsPerMinute > m_highScore )
	{
		m_highScore = m_pointsPerMinute;
	}

	GEngine->AddOnScreenDebugMessage( -1, 1.f, FColor::Red, TEXT("Points Per Minute:" + FString::SanitizeFloat( m_pointsPerMinute ) ) );
	GEngine->AddOnScreenDebugMessage( -1, 1.f, FColor::Red, TEXT("Highest Points Per Minute:" + FString::SanitizeFloat( m_highScore ) ) );
	GEngine->AddOnScreenDebugMessage( -1, 1.f, FColor::Red, TEXT("Highscore:" + FString::SanitizeFloat( m_globalScore ) ) );
}
