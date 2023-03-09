// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkTypes.h"
#include "UObject/Object.h"
#include "ZenoLandscapeEditorObject.generated.h"

UCLASS()
class ZENOLIVELINK_API UZenoLandscapeEditorObject : public UObject
{
	GENERATED_BODY()

	UPROPERTY(Category="Zeno Target", EditAnywhere, NonTransactional, meta=(DisplayName="Target Subject", ShowForTools="ImportHeightfield,ExportWeightmap", ShowForTargetTypes = "Heightmap,Visibility"))
	FLiveLinkSubjectKey SelectedSubjectKey;

	// New Landscape
	
	// The scale of the new landscape. This is the distance between each vertex on the landscape, defaulting to 100 units.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Scale", ShowForTools="ImportHeightfield"))
	FVector ImportLandscape_Scale { 100.0, 100.0, 30.0 };
	
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Enable Layer", ShowForTools="ImportHeightfield"))
	bool bImportLandscape_CanHaveLayer = true;

	// Material initially applied to the landscape. Setting a material here exposes properties for setting up layer info based on the landscape blend nodes in the material.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Material", ShowForTools="ImportHeightfield"))
	TWeakObjectPtr<UMaterialInterface> ImportLandscape_MaterialInterface = nullptr;

	// The number of quads in a single landscape section. One section is the unit of LOD transition for landscape rendering.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Section Size", ShowForTools="ImportHeightfield"))
	int32 ImportLandscape_QuadsPerSection = 7;

	// The number of sections in a single landscape component. This along with the section size determines the size of each landscape component. A component is the base unit of rendering and culling.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Section Per Component", ShowForTools="ImportHeightfield"))
	int32 ImportLandscape_SectionsPerComponent = 1;
	
	// The number of components in the X and Y direction, determining the overall size of the landscape.
	UPROPERTY(Category="New Landscape", EditAnywhere, NonTransactional, meta=(DisplayName="Number of Components", ShowForTools="ImportHeightfield"))
	FIntPoint ImportLandscape_ComponentCount = { 8, 8 };
	
	// The location of the new landscape
	UPROPERTY(Category="New Landscape", EditAnywhere, meta=(DisplayName="Location", ShowForTools="ImportHeightfield"))
	FVector NewLandscape_Location = FVector::ZeroVector;

	// The rotation of the new landscape
	UPROPERTY(Category="New Landscape", EditAnywhere, meta=(DisplayName="Rotation", ShowForTools="ImportHeightfield"))
	FRotator NewLandscape_Rotation = FRotator::ZeroRotator;
	
	friend class UZenoLandscapeTool;
};
