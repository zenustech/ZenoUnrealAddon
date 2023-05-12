// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZenoPCGVolumeComponent.generated.h"


class UZenoInputParameter;
class UZenoGraphAsset;

namespace zeno::remote
{
	struct HeightField;
}

UCLASS(ClassGroup=(Zeno), meta=(BlueprintSpawnableComponent, PrioritizeCategories = "PCG"), BlueprintType)
class ZENOLIVELINKRUNTIME_API UZenoPCGVolumeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZenoPCGVolumeComponent(const FObjectInitializer& InObjectInitializer);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	TSharedPtr<zeno::remote::HeightField> GetLandscapeHeightData() const;

protected:
#if WITH_EDITORONLY_DATA
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno, DisplayName = "PCG Graph")
	TSoftObjectPtr<UZenoGraphAsset> ZenoGraph;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Zeno, DisplayName = "Input Data")
	TArray<UZenoInputParameter*> InputParameters;

	friend class FZenoPCGActorDetailCustomization;
#endif // WITH_EDITORONLY_DATA
};
