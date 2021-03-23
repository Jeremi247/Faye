#pragma once

#include "CoreMinimal.h"

class UBoidGroupsController;
class ABoid;

class FAYE_API BoidGroup : public TSharedFromThis< BoidGroup >
{
public:
	BoidGroup( int32 size, UBoidGroupsController* owner );
	~BoidGroup();

	bool RegisterBoid( ABoid* boid );
	bool UnregisterBoid( ABoid* boid );
	void SetMaxGroupSize( int32 maxSize );

	bool IsFull();
	int32 GetRegisteredBoidsQuantity();
	bool CanBeDespawned( const FVector& m_playerPosition, float m_visibleDistance );
	void Despawn();

	void Tick();

	const FVector& GetGroupCenter();

#ifdef UE_BUILD_DEBUG
	FColor GroupColor;
#endif
private:
	void VerifyGroup();
	void UpdateGroupCenter();

	UBoidGroupsController* m_owner;
	int32 m_maxSize;
	TArray< ABoid* > m_registeredBoids;

	FVector m_groupCenter;
};
