// Zenustech © All Right Reserved

#pragma once

#include "CoreMinimal.h"
#include "LandscapeProxy.h"
#include "GameFramework/Actor.h"
#include "ZenoLandscapeBundleActor.generated.h"

class AZenoFoliageActor;
class UZenoPointSetAsset;
class ALandscapeProxy;
class UZenoLandscapeAsset;

const static FString LandscapeLabelName = TEXT("Zeno Landscape");

UCLASS()
class ZENOENGINE_API AZenoLandscapeBundleActor : public AActor
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<ALandscapeProxy>> OwnedLandscapes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSoftObjectPtr<AZenoFoliageActor>> OwnedFoliage;

public:
	void AddOwned(const ALandscapeProxy* InLandscapeData);

	void AddOwned(const AZenoFoliageActor* InPointSetData);

	/** Return the first valid landscape proxy, nullable */
	ALandscapeProxy* GetFirstLandscape();
};
