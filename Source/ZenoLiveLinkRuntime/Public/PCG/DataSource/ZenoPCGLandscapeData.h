// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoPCGSurfaceData.h"
#include "ZenoPCGLandscapeData.generated.h"

class UZenoPCGLandscapeCache;
class ULandscapeInfo;
class ALandscapeProxy;
/**
 * 
 */
UCLASS(BlueprintType, ClassGroup = (Zeno))
class ZENOLIVELINKRUNTIME_API UZenoPCGLandscapeData : public UZenoPCGSurfaceData
{
	GENERATED_BODY()

public:
	void Initialize(const TArray<TWeakObjectPtr<ALandscapeProxy>>& InLandscapes, const FBox& InBounds, bool bInHeightOnly = true, bool bInUseMetadata = false);
	
	// ~Begin UObject interface
	virtual void PostLoad() override;
	// ~End UObject interface
	
	// ~Begin UZenoPCGData interface
	virtual EZenoPCGDataType GetDataType() const override { return EZenoPCGDataType::Landscape; }
	// ~End UZenoPCGData interface

	// ~Begin UZenoPGCSpatialData interface
	virtual FBox GetBounds() const override;
	virtual FBox GetStrictBounds() const override;
	virtual bool SamplePoint(const FTransform& InTransform, const FBox& InBounds, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const override;
	virtual bool ProjectPoint(const FTransform& InTransform, const FBox& InBounds, const FZenoPCGProjectionParams& InParams, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const override;
	// ~End UZenoPCGSpatialData interface

protected:
	UPROPERTY()
	TArray<TSoftObjectPtr<ALandscapeProxy>> Landscapes;

	UPROPERTY()
	FBox Bounds = FBox(ForceInit);

	UPROPERTY()
	bool bHeightOnly = false;

	UPROPERTY()
	bool bUseMetadata = true;

	const ULandscapeInfo* GetLandscapeInfo(const FVector& InPosition) const;
	
private:
	TArray<TPair<FBox, ULandscapeInfo*>> LandscapeInfos;

	TWeakObjectPtr<UZenoPCGLandscapeCache> LandscapeCache = nullptr;
	
};
