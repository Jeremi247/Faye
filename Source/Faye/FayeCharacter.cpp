#include "FayeCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Boids/Boid.h"
#include <Runtime/Engine/Classes/Components/BoxComponent.h>
#include <Runtime/Engine/Classes/Components/SphereComponent.h>
#include "DrawDebugHelpers.h"
#include "FayeGameMode.h"
#include "ScoreManager.h"

AFayeCharacter::AFayeCharacter()
	: m_repulsionForce( 40000000.f )
	, m_isRepulsiveForceEnabled( false )
{
	//pre generated
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	m_cameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	m_cameraBoom->SetupAttachment(RootComponent);
	m_cameraBoom->SetUsingAbsoluteRotation(true);
	m_cameraBoom->TargetArmLength = 800.f;
	m_cameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	m_cameraBoom->bDoCollisionTest = false; 

	m_topDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	m_topDownCameraComponent->SetupAttachment( m_cameraBoom, USpringArmComponent::SocketName);
	m_topDownCameraComponent->bUsePawnControlRotation = false;

	m_cursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	m_cursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		m_cursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	m_cursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	m_cursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	// end pre generated

	m_repulsionArea = CreateDefaultSubobject< UBoxComponent >( TEXT( "RepulsionArea" ) );
	m_repulsionArea->SetupAttachment( RootComponent );

	m_visibilityArea = CreateDefaultSubobject< USphereComponent >( TEXT( "VisibilityArea" ) );
	m_visibilityArea->SetupAttachment( RootComponent );
	m_visibilityArea->SetCollisionEnabled( ECollisionEnabled::NoCollision );

	m_consumptionArea = CreateDefaultSubobject< UBoxComponent >( TEXT( "ConsumptionArea" ) );
	m_consumptionArea->SetupAttachment( RootComponent );
}

void AFayeCharacter::BeginPlay()
{
	Super::BeginPlay();

	AFayeGameMode* gameMode = Cast< AFayeGameMode >( GetWorld()->GetAuthGameMode() );
	m_scoreManager = gameMode->GetScoreManager();

	m_repulsionArea->OnComponentBeginOverlap.AddDynamic( this, &AFayeCharacter::OnOverlap );
	m_repulsionArea->OnComponentEndOverlap.AddDynamic( this, &AFayeCharacter::OnOverlapEnd );
}

void AFayeCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if ( m_cursorToWorld != nullptr)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			m_cursorToWorld->SetWorldLocation(TraceHitResult.Location);
			m_cursorToWorld->SetWorldRotation(CursorR);
		}
	}
}

void AFayeCharacter::EnableRepulsiveForce( bool value )
{
	m_isRepulsiveForceEnabled = value;

	for( int32 i = 0; i < m_nearbyBoids.Num(); ++i )
	{
		const TWeakObjectPtr< ABoid >& nearbyBoid = m_nearbyBoids[i];

		if( !nearbyBoid.IsValid() )
		{
			m_nearbyBoids.RemoveAtSwap( i );
			--i;
			continue;
		}

		ApplyRepulsionForce( nearbyBoid );
	}
}

bool AFayeCharacter::TryAttack()
{
	TSet< AActor* > overlappingActors;
	m_consumptionArea->GetOverlappingActors( overlappingActors );

	for( AActor* actor : overlappingActors )
	{
		if( ABoid* boid = Cast< ABoid >( actor ) )
		{
			if( !boid->HasAnyNeighbours() )
			{
				Consume( boid );
				return true;
			}
		}
	}

	return false;
}

float AFayeCharacter::GetVisibilityRadius()
{
	return m_visibilityArea->GetScaledSphereRadius();
}

void AFayeCharacter::Consume( const TWeakObjectPtr< ABoid >& boid )
{
	if( boid.IsValid() )
	{
		boid->Destroy();
		ensureAlways( m_scoreManager.IsValid() );
		m_scoreManager->AddScore( 1.f );
	}
}

void AFayeCharacter::OnOverlap(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult )
{
	if( ABoid* other = Cast< ABoid >( OtherActor ) )
	{
		if( !m_isRepulsiveForceEnabled )
		{
			m_nearbyBoids.AddUnique( other );
		}
		else
		{
			ApplyRepulsionForce( other );
		}
	}
}

void AFayeCharacter::OnOverlapEnd(
	UPrimitiveComponent* OverlappedComp,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex )
{
	if( ABoid* other = Cast< ABoid >( OtherActor ) )
	{
		m_nearbyBoids.Remove( other );
	}
}

void AFayeCharacter::ApplyRepulsionForce( const TWeakObjectPtr< ABoid >& target )
{
	ensureAlways( target.IsValid() );
	TWeakObjectPtr< UMovementComponent > movComp = target->GetMovementComponent();
	movComp->StopMovementImmediately();
	movComp->AddRadialForce( GetActorLocation(), m_repulsionArea->GetScaledBoxExtent().Size() * 2.f, m_repulsionForce, ERadialImpulseFalloff::RIF_Constant );
	target->ApplyStun();
}