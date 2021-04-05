#include "BoidGroupsController.h"
#include "BoidGroup.h"
#include "Boid.h"
#include "../FayeCharacter.h"
#include "../FayeGameMode.h"
#include "DrawDebugHelpers.h"
#pragma optimize("",off)
UBoidGroupsController::UBoidGroupsController()
{
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UBoidGroupsController::SetMaxGroupSize( int32 newSize )
{
    check( newSize > 0 );
    if( newSize <= 0 )
    {
        return;
    }

    m_maxGroupSize = newSize;

    for( const TSharedPtr< BoidGroup >& group : m_groups )
    {
        group->SetMaxGroupSize( newSize );
    }
}

void UBoidGroupsController::SetBoidsSoftLimit( int32 newLimit )
{
    m_boidsSoftLimit = newLimit;
}

void UBoidGroupsController::BeginPlay()
{
    Super::BeginPlay();

    m_player = Cast< AFayeCharacter >( GetWorld()->GetFirstPlayerController()->GetPawn() );

    AFayeGameMode* gameMode = Cast< AFayeGameMode >( GetWorld()->GetAuthGameMode() );
    TWeakObjectPtr< UDifficultyManager > diffManager = gameMode->GetDifficultyManager();
    diffManager->OnDifficultyChanged.AddDynamic( this, &UBoidGroupsController::OnDifficultyChanged );
    OnDifficultyChanged( diffManager->GetCurrentDifficulty() );
}

void UBoidGroupsController::TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
    Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

    //TO DO: Add pooling mechanism to not respawn boids but to reuse them
    VerifyGroupsCount();
    AssignBoids();
    RefillGroups();
    DestroyGrouplessBoids();

    //TO DO: Dispatch ticks on separate threads
    for( const TSharedPtr< BoidGroup >& group : m_groups )
    {
        group->Tick();
    }
}

void UBoidGroupsController::VerifyGroupsCount()
{
    int32 targetGroupCount = (m_boidsSoftLimit / m_maxGroupSize) + 1;
    int32 groupCountDiff = m_groups.Num() - targetGroupCount;

    //TO DO: find least visible groups and despawn those instead of random ones
    while( groupCountDiff > 0 )
    {
        --groupCountDiff;
        m_groups.Pop()->Despawn();
    }

    m_groups.Reserve( targetGroupCount );

    while( groupCountDiff < 0 )
    {
        ++groupCountDiff;
        m_groups.Emplace( new BoidGroup( m_maxGroupSize, this ) );
    }
}

//refills only one group at a time, can be bottlenecked if target group has no safe spawn points
void UBoidGroupsController::RefillGroups()
{
    if( m_unassignedBoids.Num() > 0 )
    {
        return;
    }

    for( const TSharedPtr< BoidGroup >& group : m_groups )
    {
        if( !group->IsFull() )
        {
            int32 registeredBoids = group->GetRegisteredBoidsQuantity();
            int32 missingBoids = m_maxGroupSize - registeredBoids - m_unassignedBoids.Num();

            FVector foundPosition;

            if( FindSafeSpawnPosition( group->GetGroupCenter(), foundPosition ) )
            {
                FActorSpawnParameters spawnParams;

                FVector groupCenterDirection = group->GetGroupCenter() - foundPosition;
                groupCenterDirection.Z = 0;
                groupCenterDirection = groupCenterDirection.GetSafeNormal();
                FRotator rotation = groupCenterDirection.ToOrientationRotator();

                for( int32 i = 0; i < missingBoids; i++ )
                {
                    ABoid* newBoid = GetWorld()->SpawnActor<ABoid>( m_boidSpawnClass, foundPosition, rotation );
                    newBoid->SpawnDefaultController();
                    m_unassignedBoids.Push( std::move( newBoid ) );
                }
            }

            return;
        }
    }
}

bool UBoidGroupsController::FindSafeSpawnPosition( const FVector& groupPosition, FVector& outPosition )
{
    FVector playerPosition( 0, 0, 200 ); //200 for simulation purposes when player doesn't exist to be above ground
    float minimumDistance = 0.f;

    if( m_player.IsValid() )
    {
        playerPosition = m_player->GetActorLocation();
        minimumDistance = m_player->GetVisibilityRadius();
    }

    FVector spawnPosition = groupPosition - playerPosition;
    spawnPosition.Z = 0;

    if( spawnPosition.Size() < minimumDistance )
    {
        if( spawnPosition.Size() == 0 )
        {
            spawnPosition.X = FMath::FRand() * 2 - 1; //random in range -1,1
            spawnPosition.Y = FMath::FRand() * 2 - 1; //random in range -1,1
        }

        spawnPosition.Normalize();

        spawnPosition *= minimumDistance;
    }

    spawnPosition.Z = playerPosition.Z;

    FHitResult traceResult;

    FVector traceStart = spawnPosition;
    FVector traceEnd = spawnPosition;
    traceEnd.Z -= m_raycastSearchLength;

    GetWorld()->LineTraceSingleByObjectType( traceResult, traceStart, traceEnd, FCollisionObjectQueryParams( ECC_TO_BITFIELD( ECC_WorldStatic ) ) );

    if( traceResult.IsValidBlockingHit() )
    {
        outPosition = traceResult.Location;
        outPosition.Z += m_boidSpawnHeight;

        return true;
    }

    return false;
}

void UBoidGroupsController::OnDifficultyChanged( const FDifficultyWrapper& newDifficulty )
{
    SetMaxGroupSize( newDifficulty.MaxBoidGroupSize );
    SetBoidsSoftLimit( newDifficulty.SoftBoidLimit );
}

void UBoidGroupsController::RequestAssign( const TWeakObjectPtr< ABoid >& boid )
{
    if( boid.IsValid() )
    {
        ensureAlways( !boid->GetCurrentGroup() );
        m_unassignedBoids.AddUnique( boid );
    }
}

void UBoidGroupsController::AssignBoids()
{
    //TO DO: split groups into empty and full separately to increase assigning performance
    for( int32 i = 0; i < m_unassignedBoids.Num(); ++i )
    {
        const TWeakObjectPtr< ABoid >& boid = m_unassignedBoids[i];
        if( !boid.IsValid() )
        {
            m_unassignedBoids.RemoveAtSwap( i );
            --i;
            continue;
        }

        if( !boid->HasActorBegunPlay() || !boid->IsActorInitialized() )
        {
            continue;
        }

        if( boid->IsActorBeingDestroyed() )
        {
            m_unassignedBoids.RemoveAtSwap( i );
            --i;
            continue;
        }

        bool foundGroup = false;

        for( const TSharedPtr< BoidGroup >& boidGroup : m_groups )
        {
            if( boidGroup->RegisterBoid( boid ) )
            {
                foundGroup = true;
                break;
            }
        }

        if( foundGroup )
        {
            m_unassignedBoids.RemoveAtSwap( i );
            --i;
        }
    }
}

void UBoidGroupsController::DestroyGrouplessBoids()
{
    for( const TSharedPtr< BoidGroup >& group : m_groups )
    {
        if( !group->IsFull() )
        {
            return;
        }
    }

    for( int32 i = 0; i < m_unassignedBoids.Num(); ++i )
    {
        if( m_unassignedBoids[i]->CanDespawnSafely( m_player ) )
        {
            m_unassignedBoids[i]->Destroy();
        }
    }

    m_unassignedBoids.Reset();
}