// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LiveLinkTypes.h"
#include "ZenoLandscapeObject.generated.h"

UCLASS()
class ZENOEDITOR_API UZenoLandscapeObject : public UObject
{
	GENERATED_BODY()

public:
	UZenoLandscapeObject();

	// Pick a subject from zeno
	UPROPERTY(Category=Zeno, EditAnywhere, NonTransactional, meta=(DisplayName="Target Subject", ShowForModes="Import"))
	FLiveLinkSubjectKey SelectedSubjectKey;
	
	// New Landscape
	// The scale of the new landscape. This is the distance between each vertex on the landscape, defaulting to 100 units.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Scale", ShowForModes="Import"))
	FVector ImportLandscape_Scale { 100.0, 100.0, 30.0 };
	
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Enable Layer", ShowForModes="Import"))
	bool bImportLandscape_CanHaveLayer = true;

	// Material initially applied to the landscape. Setting a material here exposes properties for setting up layer info based on the landscape blend nodes in the material.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Material", ShowForModes="Import"))
	TWeakObjectPtr<UMaterialInterface> ImportLandscape_MaterialInterface = nullptr;

	// The number of quads in a single landscape section. One section is the unit of LOD transition for landscape rendering.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Section Size", ShowForModes="Import"))
	int32 ImportLandscape_QuadsPerSection = 7;

	// The number of sections in a single landscape component. This along with the section size determines the size of each landscape component. A component is the base unit of rendering and culling.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Section Per Component", ShowForModes="Import"))
	int32 ImportLandscape_SectionsPerComponent = 1;
	
	// The number of components in the X and Y direction, determining the overall size of the landscape.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Number of Components", ShowForModes="Import"))
	FIntPoint ImportLandscape_ComponentCount = { 8, 8 };
	
	// The location of the new landscape
	UPROPERTY(Category="New Landscape", EditAnywhere, meta=(DisplayName="Location", ShowForModes="Import"))
	FVector NewLandscape_Location = FVector::ZeroVector;

	// The rotation of the new landscape
	UPROPERTY(Category="New Landscape", EditAnywhere, meta=(DisplayName="Rotation", ShowForModes="Import"))
	FRotator NewLandscape_Rotation = FRotator::ZeroRotator;
};
