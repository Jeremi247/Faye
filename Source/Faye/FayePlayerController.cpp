#include "FayePlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "FayeCharacter.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"

AFayePlayerController::AFayePlayerController()
	: m_dashImpulseStrength( 10000.f )
	, m_dashCooldown( 5.f )
	, m_attackCooldown( 0.5f )
	, m_currentDashCooldown( 0.f )
	, m_controlledPawn( nullptr )
	, m_pawnMovementComponent( nullptr )
	, m_isDashInProgress( false )
	, m_moveToMouseCursor( false )
{
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AFayePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if( AFayeCharacter* myPawn = Cast<AFayeCharacter>( GetPawn() ) )
	{
		m_controlledPawn = myPawn;
		ensure( m_controlledPawn );

		m_pawnMovementComponent = m_controlledPawn->GetMovementComponent();
		ensure( m_pawnMovementComponent );
	}
}

void AFayePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	HandleMovement( DeltaTime );

	m_currentAttackCooldown -= DeltaTime;
}

void AFayePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("SetDestination", IE_Pressed, this, &AFayePlayerController::OnSetDestinationPressed);
	InputComponent->BindAction("SetDestination", IE_Released, this, &AFayePlayerController::OnSetDestinationReleased);

	InputComponent->BindAction( "Dash", IE_Pressed, this, &AFayePlayerController::OnDashPressed );
	InputComponent->BindAction( "Attack", IE_Pressed, this, &AFayePlayerController::OnAttackPressed );
}

void AFayePlayerController::HandleMovement( float deltaTime )
{
	if( m_dashTime >= 0.1 && m_controlledPawn->GetVelocity().Size() <= m_pawnMovementComponent->GetMaxSpeed() )
	{
		m_dashTime = 0.f;
		m_isDashInProgress = false;
		m_controlledPawn->EnableRepulsiveForce( false );
	}

	if( m_moveToMouseCursor && !m_isDashInProgress )
	{
		MoveToMouseCursor();
	}

	if( m_isDashInProgress )
	{
		m_dashTime += deltaTime;
	}

	m_currentDashCooldown -= deltaTime;
}

void AFayePlayerController::MoveToMouseCursor()
{
	if ( m_controlledPawn->GetCursorToWorld())
	{
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, m_controlledPawn->GetCursorToWorld()->GetComponentLocation());
	}
}

void AFayePlayerController::OnSetDestinationPressed()
{
	m_moveToMouseCursor = true;
}

void AFayePlayerController::OnSetDestinationReleased()
{
	m_moveToMouseCursor = false;
}

void AFayePlayerController::OnDashPressed()
{
	if( m_currentDashCooldown < 0.f && !m_isDashInProgress)
	{
		m_controlledPawn->EnableRepulsiveForce( true );
		m_isDashInProgress = true;
		m_currentDashCooldown = m_dashCooldown;

		m_pawnMovementComponent->StopMovementImmediately();

		FVector behindPlayerLocation = m_controlledPawn->GetActorLocation() - m_controlledPawn->GetActorForwardVector() * 10.f;
		m_pawnMovementComponent->AddRadialImpulse( behindPlayerLocation, 300.f, m_dashImpulseStrength, ERadialImpulseFalloff::RIF_Constant , true );
	}
}

void AFayePlayerController::OnAttackPressed()
{
	if( m_currentAttackCooldown < 0.f )
	{
		if( m_controlledPawn->TryAttack() )
		{
			m_currentAttackCooldown = m_attackCooldown;
		}
	}
}