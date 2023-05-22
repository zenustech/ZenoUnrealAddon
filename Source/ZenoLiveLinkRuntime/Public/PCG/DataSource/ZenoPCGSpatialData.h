// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoPCGData.h"
#include "ZenoPCGSpatialData.generated.h"

class UZenoMetadata;

/**
 * Spatial data for procedural content generation.
 */
UCLASS(Abstract, BlueprintType, ClassGroup = (Zeno))
class ZENOLIVELINKRUNTIME_API UZenoPCGSpatialData : public UZenoPCGData
{
	GENERATED_BODY()

public:
	UZenoPCGSpatialData(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UObject Interface
	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;
	//~End UObject Interface

	// ~Begin UZenoPCGData interface
	virtual EZenoPCGDataType GetDataType() const override { return EZenoPCGDataType::Spatial; }
	// ~End UZenoPCGData interface

	/** Get the dimension of this data source. */
	UFUNCTION(BlueprintCallable, Category=SpatialData)
	virtual int32 GetDimension() const PURE_VIRTUAL(UZenoPCGSpatialData::GetDimension(), return 0;);

	/** Sample a point from this data source with lerp. */
	UFUNCTION(BlueprintCallable, Category=SpatialData)
	virtual bool SamplePoint(const FTransform& InTransform, const FBox& Bounds, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const PURE_VIRTUAL(UZenoPCGSpatialData::SamplePoint, return false;);

	UPROPERTY(VisibleAnywhere, Category=Metadata)
	TObjectPtr<UZenoMetadata> Metadata = nullptr;

};

UCLASS(Abstract, ClassGroup = (Zeno))
class ZENOLIVELINKRUNTIME_API UZenoPCGSpatialDataWithPointCache : public UZenoPCGSpatialData
{
	GENERATED_BODY()

public:
	//~Begin UObject Interface
	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;
	//~End UObject Interface

private:
	mutable FCriticalSection CacheLock;
};
