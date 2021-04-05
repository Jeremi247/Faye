#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../DifficultyManager.h"
#include "BoidGroupsController.generated.h"

class ABoid;
class BoidGroup;
class AFayeCharacter;

UCLASS( ClassGroup = (Custom) )
class FAYE_API UBoidGroupsController : public UActorComponent
{
	GENERATED_BODY()

public:
	UBoidGroupsController();

	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	void RequestAssign( const TWeakObjectPtr< ABoid >& boid );
	void NotifyNewDifficulty( const FDifficultyWrapper& newDifficulty );

private:
	UFUNCTION()
	void OnDifficultyChanged( const FDifficultyWrapper& newDifficulty );

	void SetMaxGroupSize( int32 newSize );
	void SetBoidsSoftLimit( int32 newLimit );
	void VerifyGroupsCount();
	void RefillGroups();
	void AssignBoids();
	void DestroyGrouplessBoids();
	bool FindSafeSpawnPosition( const FVector& groupPosition, FVector& outPosition );

	UPROPERTY( EditAnywhere, Category = "Boid Spawn", DisplayName = "Boid Class" )
	TSubclassOf< ABoid > m_boidSpawnClass;
	UPROPERTY( EditAnywhere, Category = "Boid Spawn", DisplayName = "Boid Spawn Height" )
	float m_boidSpawnHeight;
	UPROPERTY( EditAnywhere, Category = "Boid Spawn", DisplayName = "Raycast Search Length" )
	float m_raycastSearchLength;

	int32 m_maxGroupSize;
	int32 m_boidsSoftLimit;

	TArray< TSharedPtr< BoidGroup > > m_groups;
	TArray< TWeakObjectPtr< ABoid > > m_unassignedBoids;

	TWeakObjectPtr< AFayeCharacter > m_player;
};
