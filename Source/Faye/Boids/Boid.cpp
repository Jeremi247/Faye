#include "Boid.h"
#include <Runtime/Engine/Classes/Components/SphereComponent.h>
#include "../Boids/BoidGroup.h"
#include "../Boids/BoidGroupsController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../FayeGameMode.h"
#include "../FayeCharacter.h"
#include "DrawDebugHelpers.h"

ABoid::ABoid( const FObjectInitializer& OI )
	: Super( OI )
	, m_minSpeed(300.f)
	, m_maxSpeed(1000.f)
	, m_maxSteeringForce(1000.f)
	, m_flockForwardWeight( 2.f )
	, m_avoidanceWeight( 2.f )
	, m_groupDirectionWeight( 1.f )
	, m_avoidanceRadiusPercent( 0.6f )
	, m_enableDebugDraw( true )
	, m_stunLength( 2.f )
	, m_stunRotationSpeed( 1000.f )
	, m_overlappingObstacles( 0 )
	, m_currentStunLength( 0.f )
{
#ifdef UE_BUILD_DEBUG
	PrimaryActorTick.bCanEverTick = true;
#endif
	GetCharacterMovement()->bOrientRotationToMovement = true;

	NeighbourDetection = CreateDefaultSubobject<USphereComponent>( TEXT( "SphereComponent" ) );
	NeighbourDetection->SetSphereRadius( 300.f );
	NeighbourDetection->SetupAttachment( RootComponent );
}

void ABoid::BeginPlay()
{
	CacheComponents();

	AFayeGameMode* gameMode = GetWorld()->GetAuthGameMode< AFayeGameMode >();
	UDifficultyManager* difficultyManager = gameMode->GetDifficultyManager();
	difficultyManager->OnDifficultyChanged.AddDynamic( this, &ABoid::OnDifficultyChanged );
	OnDifficultyChanged( difficultyManager->GetCurrentDifficulty() );

	NeighbourDetection->OnComponentBeginOverlap.AddDynamic( this, &ABoid::OnOverlap );
	NeighbourDetection->OnComponentEndOverlap.AddDynamic( this, &ABoid::OnOverlapEnd );

	// example angle 270
	// 270 / 180 = 1.5
	// 2 - 1.5 = 0.5
	// -1 + 0.5 = -0.5 <- this is value to compare with dot product of other boid position
	m_cutoffViewPercent = -1.f + (2.f - m_viewAngle / 180.f);

	Super::BeginPlay();
}

void ABoid::CacheComponents()
{
	m_movementComponent = GetMovementComponent();

	AFayeGameMode* gameMode = GetWorld()->GetAuthGameMode<AFayeGameMode>();
	m_boidGroupsController = gameMode->GetBoidGroupsController();
}

void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//TO DO: Move to multithreaded call from boid group
	UpdateMovement(DeltaTime);

#ifdef UE_BUILD_DEBUG
	if( m_enableDebugDraw )
	{
		FVector location = GetActorLocation();
		location.Z += 50.f;

		FColor drawColor = FColor::Black;
		if( m_myGroup )
		{
			drawColor = m_myGroup->GroupColor;
		}

		DrawDebugSphere( GetWorld(), location, 25.f, 5, drawColor );
	}
#endif
}

void ABoid::UpdateMovement( float deltaTime )
{
	if( m_currentStunLength > 0.f )
	{
		HandleStun( deltaTime );
		return;
	}

	HandleMovement();
}

void ABoid::UpdateSteeringVectors()
{
	m_localFlockForward = FVector::ZeroVector;
	m_avoidanceDirection = FVector::ZeroVector;
	m_groupDirection = FVector::ZeroVector;

	for( ABoid* neighbour : m_neighbours )
	{
		if( neighbour->GetCurrentGroup() == m_myGroup )
		{
			FVector direction = neighbour->GetActorLocation() - GetActorLocation();
			direction.Normalize();
			float dot = FVector::DotProduct( GetActorForwardVector(), direction );

			if( dot > m_cutoffViewPercent )
			{
				m_localFlockForward += neighbour->GetActorForwardVector();
			}
#ifdef UE_BUILD_DEBUG
			if( m_enableDebugDraw )
			{
				FColor drawColor = FColor::Red;
				if( dot > m_cutoffViewPercent )
				{
					drawColor = FColor::Green;
				}

				DrawDebugLine( GetWorld(), GetActorLocation(), neighbour->GetActorLocation(), drawColor );
			}
#endif
		}

		FVector offset = neighbour->GetActorLocation() - GetActorLocation();

		if( offset.Size() < NeighbourDetection->GetScaledSphereRadius() * m_avoidanceRadiusPercent )
		{
			if( offset.Size() == 0 )
			{
				m_avoidanceDirection.X = FMath::FRand();
				m_avoidanceDirection.Y = FMath::FRand();
			}
			else
			{
				m_avoidanceDirection -= offset / offset.SizeSquared();
			}
		}
	}

	if( m_myGroup )
	{
		m_groupDirection = m_myGroup->GetGroupCenter() - GetActorLocation();
	}
}

void ABoid::HandleStun( float deltaTime )
{
	m_currentStunLength -= deltaTime;
	FVector rotation = GetActorRotation().Euler();
	rotation.Z += m_stunRotationSpeed * deltaTime;

	SetActorRotation( FRotator::MakeFromEuler( rotation ) );
}

void ABoid::HandleMovement()
{
	UpdateSteeringVectors();

	FVector acceleration = FVector::ZeroVector;
	//TO DO: Add collision avoidance
	acceleration += SteerTowards( m_localFlockForward ) * m_flockForwardWeight;
	acceleration += SteerTowards( m_avoidanceDirection ) * m_avoidanceWeight;
	acceleration += SteerTowards( m_groupDirection ) * m_groupDirectionWeight;

	FVector velocity = m_movementComponent->Velocity + (acceleration * GetWorld()->GetDeltaSeconds());
	float speed = velocity.Size();
	FVector direction = velocity / speed;
	speed = FMath::Clamp( speed, m_minSpeed, m_maxSpeed );
	velocity = direction * speed;

	m_movementComponent->RequestDirectMove( velocity, false );
}

const TSharedPtr< BoidGroup >& ABoid::GetCurrentGroup()
{
	return m_myGroup;
}

void ABoid::AddToGroup( const TSharedPtr< BoidGroup >& newGroup )
{
	ensureAlways( !m_myGroup || !newGroup );
	m_myGroup = newGroup;
}

void ABoid::ApplyStun()
{
	m_isStunned = true;
	m_currentStunLength = m_stunLength;
}

bool ABoid::CanDespawnSafely( AFayeCharacter* player )
{
	FVector distanceFromPlayer = player->GetActorLocation() - GetActorLocation();

	if( distanceFromPlayer.Size() < player->GetVisibilityRadius() )
	{
		return false;
	}

	return true;
}

bool ABoid::HasAnyNeighbours()
{
	return m_neighbours.Num() > 0;
}

void ABoid::BeginDestroy()
{
	if( m_myGroup )
	{
		verify( m_myGroup->UnregisterBoid( this ) );
	}

	Super::BeginDestroy();
}

FVector ABoid::SteerTowards( FVector vector )
{
	vector.Normalize();
	vector *= m_maxSpeed;
	vector -= m_movementComponent->Velocity;
	return vector.GetClampedToMaxSize( m_maxSteeringForce );
}

void ABoid::OnOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult )
{
	ECollisionResponse response = OtherComp->GetCollisionResponseToChannel( ECollisionChannel::ECC_WorldStatic );
	if( response == ECollisionResponse::ECR_Block )
	{
		++m_overlappingObstacles;
	}

	if( ABoid* boid = Cast<ABoid>( OtherActor ) )
	{
		if( boid != this )
		{
			m_neighbours.AddUnique( boid );
		}
	}
}

void ABoid::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex )
{
	ECollisionResponse response = OtherComp->GetCollisionResponseToChannel( ECollisionChannel::ECC_WorldStatic );
	if( response == ECollisionResponse::ECR_Block )
	{
		--m_overlappingObstacles;
	}

	if( ABoid* boid = Cast<ABoid>( OtherActor ) )
	{
		m_neighbours.Remove( boid );
	}
}

void ABoid::OnDifficultyChanged( const FDifficultyWrapper& newDifficulty )
{
	m_stunLength = newDifficulty.BoidStunTime;
}