// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZenoPCGVolumeComponent.generated.h"


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
};
