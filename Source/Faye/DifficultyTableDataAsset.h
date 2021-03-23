#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DifficultyTableDataAsset.generated.h"

USTRUCT()
struct FDifficultyWrapper
{
	GENERATED_USTRUCT_BODY()

	/** Lower difficulty level to previous one if points per minute fall bellow this bar */
	UPROPERTY( EditAnywhere )
	float MinPointsPerMinute;

	/** Rise difficulty level to this one if points per minute rise above this bar */
	UPROPERTY( EditAnywhere )
	float ReqPointsPerMinute;

	UPROPERTY( EditAnywhere )
	int32 MaxBoidGroupSize;
	
	/** All boid groups will be filled to the brim causing actual amount of boids to be higher than this limit */
	UPROPERTY( EditAnywhere )
	int32 SoftBoidLimit;

	UPROPERTY( EditAnywhere )
	float BoidStunTime;
};

UCLASS()
class FAYE_API UDifficultyTableDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TArray< FDifficultyWrapper > Difficulties;
};
