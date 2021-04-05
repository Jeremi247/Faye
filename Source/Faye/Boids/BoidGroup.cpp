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

bool BoidGroup::RegisterBoid( const TWeakObjectPtr< ABoid >& boid )
{
    if( m_registeredBoids.Num() < m_maxSize && boid.IsValid() )
    {
        m_registeredBoids.Push( boid );
        boid->AddToGroup( SharedThis( this ) );
        UpdateGroupCenter();
        return true;
    }

    return false;
}

bool BoidGroup::UnregisterBoid( const TWeakObjectPtr< ABoid >& boid )
{
    for( int32 i = 0; i < m_registeredBoids.Num(); ++i )
    {
        if( m_registeredBoids[i].Get() == boid.Get() )
        {
            m_registeredBoids.RemoveAtSwap( i );
            return true;
        }
    }

    return false;
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
        TWeakObjectPtr< ABoid > removedBoid = m_registeredBoids.Pop();
        if( removedBoid.IsValid() )
        {
            removedBoid->AddToGroup( nullptr );
            m_owner->RequestAssign( removedBoid );
        }
    }
}

void BoidGroup::UpdateGroupCenter()
{
    m_groupCenter = FVector::ZeroVector;

    int validBoidsNum = 0;

    for( int32 i = 0; i < m_registeredBoids.Num(); ++i )
    {
        const TWeakObjectPtr< ABoid >& boid = m_registeredBoids[i];
        if( !boid.IsValid() )
        {
            m_registeredBoids.RemoveAtSwap( i );
            --i;
            continue;
        }

        m_groupCenter += boid->GetActorLocation();
        ++validBoidsNum;
    }

    if( validBoidsNum > 0 )
    {
        m_groupCenter /= validBoidsNum;
    }
}