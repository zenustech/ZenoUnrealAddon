// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Landscape.h"
#include "ZenoLandscapeAnchor.generated.h"

class UZenoBridgeAssetData_Heightfield;
UCLASS()
class ZENOLIBRARY_API AZenoLandscapeAnchor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZenoLandscapeAnchor(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(Category = "Zeno|Property", BlueprintReadOnly, EditAnywhere)
	UZenoBridgeAssetData_Heightfield* Heightfield = nullptr;

	UPROPERTY(Category = "Zeno|Property", BlueprintReadOnly, EditAnywhere)
	FVector LandscapeScale = FVector{1000, 1000, 360};

	UPROPERTY()
	ALandscape* LandscapeActor;

	virtual void OnConstruction(const FTransform& Transform) override;

};
