#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../DifficultyManager.h"
#include "Boid.generated.h"

class USphereComponent;
class BoidGroup;
class UBoidGroupsController;
class AFayeCharacter;

UCLASS()
class FAYE_API ABoid : public ACharacter
{
	GENERATED_BODY()

public:
	ABoid( const FObjectInitializer& OI );

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void UpdateMovement(float deltaTime);

	const TSharedPtr< BoidGroup >& GetCurrentGroup();
	void AddToGroup( const TSharedPtr< BoidGroup >& newGroup );

	void ApplyStun();
	bool CanDespawnSafely( AFayeCharacter* player );

	bool HasAnyNeighbours();

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly )
	USphereComponent* NeighbourDetection;

private:
	void CacheComponents();
	FVector SteerTowards( FVector vector );
	void UpdateSteeringVectors();
	void HandleStun( float deltaTime );
	void HandleMovement();
	virtual void BeginDestroy() override;

	UFUNCTION()
	void OnOverlap( 
		UPrimitiveComponent* OverlappedComp, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, 
		bool bFromSweep, 
		const FHitResult& SweepResult );

	UFUNCTION()
	void OnOverlapEnd(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	UFUNCTION()
	void OnDifficultyChanged( const FDifficultyWrapper& newDifficulty );

	UPROPERTY( EditAnywhere, Category = "Speed controll", DisplayName = "Min Speed" )
	float m_minSpeed;
	UPROPERTY( EditAnywhere, Category = "Speed controll", DisplayName = "Max Speed" )
	float m_maxSpeed;
	UPROPERTY( EditAnywhere, Category = "Speed controll", DisplayName = "Max Steering Force" )
	float m_maxSteeringForce;

	UPROPERTY( EditAnywhere, Category = "Steering", DisplayName = "Flock Forward Weight" )
	float m_flockForwardWeight;
	UPROPERTY( EditAnywhere, Category = "Steering", DisplayName = "Avoidance Weight" )
	float m_avoidanceWeight;
	UPROPERTY( EditAnywhere, Category = "Steering", DisplayName = "Group Direction Weigth" )
	float m_groupDirectionWeight;
	UPROPERTY( EditAnywhere, Category = "Steering", DisplayName = "View Angle" )
	float m_viewAngle;
	UPROPERTY( EditAnywhere, Category = "Steering", DisplayName = "Avoidance Radius Percent" )
	float m_avoidanceRadiusPercent;

	UPROPERTY( EditAnywhere, Category = "Debug", DisplayName = "Enable Debug Draw" )
	bool m_enableDebugDraw;

	UPROPERTY( EditAnywhere, Category = "Misc", DisplayName = "Stun Length" )
	float m_stunLength;
	UPROPERTY( EditAnywhere, Category = "Misc", DisplayName = "Stun Rotation Speed" )
	float m_stunRotationSpeed;

	UPawnMovementComponent* m_movementComponent;

	TArray< ABoid* > m_neighbours;
	TSharedPtr< BoidGroup > m_myGroup;
	UBoidGroupsController* m_boidGroupsController;
	int32 m_overlappingObstacles;

	FVector m_localFlockForward;
	FVector m_avoidanceDirection;
	FVector m_groupDirection;

	float m_cutoffViewPercent;

	float m_currentStunLength;
	bool m_isStunned;
};
