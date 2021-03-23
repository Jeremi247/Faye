#include "BoidGroup.h"
#include "BoidGroupsController.h"
#include "Boid.h"

BoidGroup::BoidGroup( int32 size, UBoidGroupsController* owner )
    : m_owner( owner )
    , m_maxSize( size )
    , m_groupCenter( 0, 0, 0 ) 
{
#ifdef UE_BUILD_DEBUG
    GroupColor = FColor::MakeRandomColor();
#endif
}

BoidGroup::~BoidGroup()
{
}

bool BoidGroup::RegisterBoid( ABoid* boid )
{
    if( m_registeredBoids.Num() < m_maxSize )
    {
        m_registeredBoids.Push( boid );
        boid->AddToGroup( SharedThis( this ) );
        UpdateGroupCenter();
        return true;
    }

    return false;
}

bool BoidGroup::UnregisterBoid( ABoid* boid )
{
    return m_registeredBoids.Remove( boid ) > 0;
}

void BoidGroup::SetMaxGroupSize( int32 maxSize )
{
    m_maxSize = maxSize;
    VerifyGroup();
}

bool BoidGroup::IsFull()
{
    return m_registeredBoids.Num() >= m_maxSize;
}

int32 BoidGroup::GetRegisteredBoidsQuantity()
{
    return m_registeredBoids.Num();
}

void BoidGroup::Despawn()
{
    m_maxSize = 0;

    VerifyGroup();
}

void BoidGroup::Tick()
{
    UpdateGroupCenter();
}

const FVector& BoidGroup::GetGroupCenter()
{
    return m_groupCenter;
}

void BoidGroup::VerifyGroup()
{
    while( m_registeredBoids.Num() > m_maxSize )
    {
        ABoid* removedBoid = m_registeredBoids.Pop();
        removedBoid->AddToGroup( nullptr );
        m_owner->RequestAssign( removedBoid );
    }
}

void BoidGroup::UpdateGroupCenter()
{
    m_groupCenter = FVector::ZeroVector;

    int validBoidsNum = 0;

    for( ABoid* boid : m_registeredBoids )
    {
        m_groupCenter += boid->GetActorLocation();
        ++validBoidsNum;
    }

    if( validBoidsNum > 0 )
    {
        m_groupCenter /= validBoidsNum;
    }
}