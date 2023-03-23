// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LandscapeBlueprintBrush.h"
#include "ZenoLandscapeSimpleBrush.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogZenoLandscapeSimpleBrush, Log, All);

UCLASS(Config = Engine, DefaultConfig, meta=(DisplayName="Zeno Landscape"))
class ZENOLIVELINK_API UZenoSimpleBrushSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UZenoSimpleBrushSettings();
	
public:
	UPROPERTY(EditAnywhere, config, Category = Brush, AdvancedDisplay)
	TSoftObjectPtr<UMaterialInterface> DefaultBrushHeightMapEncoderMaterial;
	
};

UCLASS(Blueprintable)
class ZENOLIVELINK_API AZenoLandscapeSimpleBrush : public ALandscapeBlueprintBrush
{
	GENERATED_BODY()

public:
	explicit AZenoLandscapeSimpleBrush(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/** Set the landscape brush controlling */
	virtual void SetTargetLandscape(ALandscape* InTargetLandscape);

	/** Set height data into base height map */
	virtual void SetBaseHeightmapData(const TArray<uint16>& InHeightmapData);

protected:
	// AActor
	virtual void PostActorCreated() override;
	virtual void BeginPlay() override;
	virtual void PostLoad() override;
	// AActor

	/** Loading default material instance for our brush */
	virtual void SetupDefaultMaterials();
	/** Allocate RT resources */
	virtual bool AllocateRTs();
	/** Create material instance dynamics */
	virtual bool CreateMIDs();
	/** Setup resource and material parameters for rendering */
	virtual bool BrushRenderSetup();

public:
	// ALandscapeBlueprintBrush
	virtual void Tick(float DeltaTime) override;
	virtual UTextureRenderTarget2D* Render_Native(bool InIsHeightmap, UTextureRenderTarget2D* InCombinedResult, const FName& InWeightmapLayerName) override;
	virtual void Initialize_Native(const FTransform& InLandscapeTransform, const FIntPoint& InLandscapeSize, const FIntPoint& InLandscapeRenderTargetSize) override;
	virtual void CheckForErrors() override;
	virtual void GetRenderDependencies(TSet<UObject*>& OutDependencies) override;
	virtual bool IsAffectingWeightmapLayer(const FName& InLayerName) const override;
	// ALandscapeBlueprintBrush

protected:
	/** Keep landscape render target passed in Render_Native function */
	UPROPERTY(VisibleAnywhere, Transient, Category="Render Targets")
	TObjectPtr<UTextureRenderTarget2D> LandscapeRTRef = nullptr;

	/** Resolution of landscape's render target. Receive from Initialize_Native. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FIntPoint LandscapeRTRes;

	/** Landscape size, quads num in a landscape. Receive from Initialize_Native. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Debug")
	FIntPoint LandscapeQuads;

	/** Height data pass from CPU side */
	// UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Render Data")
	TArray<uint16> BaseHeightData;

	// Render Targets

	/** Base heightmap */
	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TObjectPtr<UTextureRenderTarget2D> BaseHeightmapRT;
	
	/** Heightmap to return the brush result */
	UPROPERTY(VisibleAnywhere, Transient, Category = "Render Targets")
	TObjectPtr<UTextureRenderTarget2D> HeightmapRTA;

	// Materials
	
	/** Material to draw Heightmap RT */
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Category = "Brush Materials")
	TObjectPtr<UMaterialInterface> BrushHeightMapEncoderMaterial;

	// Material Instances
	UPROPERTY(EditAnywhere, AdvancedDisplay, BlueprintReadWrite, Transient, Category = "Debug MIDs")
	TObjectPtr<UMaterialInstanceDynamic> BrushHeightMapEncoderMID;
};
