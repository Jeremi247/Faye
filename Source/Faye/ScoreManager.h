#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include <utility>
#include "ScoreManager.generated.h"

class UDifficultyManager;

UCLASS( ClassGroup=(Custom) )
class FAYE_API UScoreManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UScoreManager();

public:	
	void AddScore( int score );

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void ValidateScorePerMinute();

	TWeakObjectPtr< UDifficultyManager > m_difficultyManager;

	TArray< std::pair< float, float > > m_scoreTimeStamps;
	float m_globalScore;
	float m_highScore;
	float m_pointsPerMinute;
};
