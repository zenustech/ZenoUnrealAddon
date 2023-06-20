// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZenoPCGVolumeComponent.generated.h"


namespace zeno
{
	namespace remote
	{
		struct PointSet;
		struct HeightField;
	}
}

class UZenoInputParameter;
class UZenoGraphAsset;

DECLARE_MULTICAST_DELEGATE(FZenoPCGVolumeComponentParameterChangedEvent);

UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent, PrioritizeCategories = "PCG"), BlueprintType)
class ZENOLIVELINKRUNTIME_API UZenoPCGVolumeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZenoPCGVolumeComponent(const FObjectInitializer& InObjectInitializer);

	FZenoPCGVolumeComponentParameterChangedEvent ParameterChangedEvent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	std::shared_ptr<zeno::remote::HeightField> GetLandscapeHeightData() const;
	std::shared_ptr<zeno::remote::PointSet> GetScatteredPoints() const;
	
	FBox GetActorBounds() const;

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno, DisplayName = "PCG Graph")
	TSoftObjectPtr<UZenoGraphAsset> ZenoGraph;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno, DisplayName = "Input Data")
	TArray<UZenoInputParameter*> InputParameters;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno, DisplayName = "Seed")
	int32 ScatterSeed = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno, DisplayName = "Seed", meta = ( ClampMin = 0, UIMin = 0 ))
	int32 ScatterPoints = 0;

	friend class FZenoPCGActorDetailCustomization;
	friend class AZenoPCGVolume;
#endif // WITH_EDITORONLY_DATA
};
