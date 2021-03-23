#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DifficultyTableDataAsset.h"
#include "DifficultyManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE_OneParam( FDifficultyChangedSignature, UDifficultyManager, OnDifficultyChanged, const FDifficultyWrapper&, DifficultyWrapper );

UCLASS( ClassGroup=(Custom) )
class FAYE_API UDifficultyManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDifficultyManager();

	void VerifyDifficultyLevel( float pointsPerMinute );
	const FDifficultyWrapper& GetCurrentDifficulty();

	FDifficultyChangedSignature OnDifficultyChanged;

protected:
	virtual void BeginPlay() override;

private:
	void TryLowerDifficulty( int32& newDiffIndex, float pointsPerMinute );
	void TryRiseDifficulty( int32& newDiffIndex, float pointsPerMinute );
	void SetDifficultyLevel( int32 newDifficulty );

	UPROPERTY( EditAnywhere, Category = "Data", DisplayName = "Difficulties Data Asset", meta = (AllowPrivateAccess = "true") )
	UDifficultyTableDataAsset* m_difficultyDataAsset;

	int32 m_currentDifficultyIndex;
};
