#include "DifficultyManager.h"

UDifficultyManager::UDifficultyManager()
	: m_currentDifficultyIndex(-1)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDifficultyManager::BeginPlay()
{
	Super::BeginPlay();

	VerifyDifficultyLevel( 0 );
}

const FDifficultyWrapper& UDifficultyManager::GetCurrentDifficulty()
{
	if( m_currentDifficultyIndex >= 0 )
	{
		return m_difficultyDataAsset->Difficulties[m_currentDifficultyIndex];
	}
	else
	{
		return m_difficultyDataAsset->Difficulties[0];
	}
}

void UDifficultyManager::VerifyDifficultyLevel( float pointsPerMinute )
{
	int32 newDifficultyIndex = m_currentDifficultyIndex;

	if( m_currentDifficultyIndex > 0 )
	{
		TryLowerDifficulty( newDifficultyIndex, pointsPerMinute );
	}

	if( newDifficultyIndex == m_currentDifficultyIndex )
	{
		TryRiseDifficulty( newDifficultyIndex, pointsPerMinute );
	}

	if( newDifficultyIndex < 0 )
	{
		newDifficultyIndex = 0;
	}

	if( newDifficultyIndex != m_currentDifficultyIndex )
	{
		SetDifficultyLevel( newDifficultyIndex );
	}
}

void UDifficultyManager::TryLowerDifficulty( int32& newDiffIndex, float pointsPerMinute )
{
	for( int32 i = m_currentDifficultyIndex; i >= 0; --i )
	{
		const FDifficultyWrapper& currentDifficulty = m_difficultyDataAsset->Difficulties[i];

		newDiffIndex = i;

		if( pointsPerMinute >= currentDifficulty.MinPointsPerMinute )
		{
			return;
		}
	}
}

void UDifficultyManager::TryRiseDifficulty( int32& newDiffIndex, float pointsPerMinute )
{
	for( int32 i = m_currentDifficultyIndex + 1; i < m_difficultyDataAsset->Difficulties.Num(); ++i )
	{
		const FDifficultyWrapper& currentDifficulty = m_difficultyDataAsset->Difficulties[i];

		if( pointsPerMinute < currentDifficulty.ReqPointsPerMinute )
		{
			return;
		}

		newDiffIndex = i;
	}
}

void UDifficultyManager::SetDifficultyLevel( int32 newDifficulty )
{
	m_currentDifficultyIndex = newDifficulty;
	OnDifficultyChanged.Broadcast( m_difficultyDataAsset->Difficulties[newDifficulty] );
}