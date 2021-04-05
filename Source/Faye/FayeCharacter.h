#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FayeCharacter.generated.h"

class UBoxComponent;
class USphereComponent;
class UCameraComponent;
class USpringArmComponent;
class UDecalComponent;
class UScoreManager;
class ABoid;

UCLASS(Blueprintable)
class AFayeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFayeCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return m_topDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return m_cameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return m_cursorToWorld; }

	void EnableRepulsiveForce( bool value );
	bool TryAttack();

	float GetVisibilityRadius();

private:	
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
		int32 OtherBodyIndex );

	void ApplyRepulsionForce( const TWeakObjectPtr< ABoid >& target );
	void Consume( const TWeakObjectPtr< ABoid >& boid );

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Repulsion, DisplayName = "Repulsion Area", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* m_repulsionArea;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Misc, DisplayName = "Consumption Area", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* m_consumptionArea;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Misc, DisplayName = "Visibility Area", meta = (AllowPrivateAccess = "true"))
	USphereComponent* m_visibilityArea;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, DisplayName = "Top Down Camera Component", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* m_topDownCameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, DisplayName = "Camera Boom", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* m_cameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, DisplayName = "Cursors To World", meta = (AllowPrivateAccess = "true"))
	UDecalComponent* m_cursorToWorld;

	UPROPERTY( EditAnywhere, Category = Repulsion, DisplayName = "Repulsion Force" )
	float m_repulsionForce;

	TWeakObjectPtr< UScoreManager > m_scoreManager;
	TArray< TWeakObjectPtr< ABoid > > m_nearbyBoids;
	bool m_isRepulsiveForceEnabled;
};

