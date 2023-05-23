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

	/** Returns the full bounds (including density fall-off) of the data */
	UFUNCTION(BlueprintCallable, Category = SpatialData)
	virtual FBox GetBounds() const PURE_VIRTUAL(UPCGSpatialData::GetBounds, return FBox(EForceInit::ForceInit););

	/** Returns whether a given spatial data is bounded as some data types do not require bounds by themselves */
	virtual bool IsBounded() const { return true; }

	/** Returns the bounds in which the density is always 1 */
	UFUNCTION(BlueprintCallable, Category = SpatialData)
	virtual FBox GetStrictBounds() const { return FBox(EForceInit::ForceInit); }
	
	/** Returns the expected data normal (for surfaces) or eventual projection axis (for volumes) */
	UFUNCTION(BlueprintCallable, Category = SpatialData)
	virtual FVector GetNormal() const { return FVector::UnitZ(); }

	/** Sample a point from this data source with lerp. */
	UFUNCTION(BlueprintCallable, Category=SpatialData)
	virtual bool SamplePoint(const FTransform& InTransform, const FBox& Bounds, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const PURE_VIRTUAL(UZenoPCGSpatialData::SamplePoint, return false;);
	
	/** Project the query point onto this data, and sample point and metadata information at the projected position. Returns true if successful. */
	UFUNCTION(BlueprintCallable, Category = SpatialData)
	virtual bool ProjectPoint(const FTransform& InTransform, const FBox& InBounds, const FZenoPCGProjectionParams& InParams, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const;
	
	UPROPERTY(Transient, BlueprintReadWrite, EditAnywhere, Category = Data)
	TWeakObjectPtr<AActor> TargetActor = nullptr;

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
