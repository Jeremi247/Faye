#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FayePlayerController.generated.h"

class AFayeCharacter;
class UMovementComponent;

UCLASS()
class AFayePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFayePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	void HandleMovement( float deltaTime );
	void MoveToMouseCursor();

	void OnSetDestinationPressed();
	void OnSetDestinationReleased();
	void OnDashPressed();
	void OnAttackPressed();

	UPROPERTY( EditAnywhere, Category = "Dash", DisplayName = "Dash Impulse Strength" )
	float m_dashImpulseStrength;
	UPROPERTY( EditAnywhere, Category = "Dash", DisplayName = "Dash Cooldown" )
	float m_dashCooldown;
	UPROPERTY( EditAnywhere, Category = "Attack", DisplayName = "Attack Cooldown" )
	float m_attackCooldown;

	float m_currentDashCooldown;
	float m_currentAttackCooldown;

	float m_dashTime;

	AFayeCharacter* m_controlledPawn;
	UMovementComponent* m_pawnMovementComponent;

	bool m_isDashInProgress;
	bool m_moveToMouseCursor;
};


