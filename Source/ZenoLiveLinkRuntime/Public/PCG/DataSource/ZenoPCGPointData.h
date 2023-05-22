// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoPCGSpatialData.h"
#include "ZenoPCGPointData.generated.h"

UCLASS(BlueprintType, ClassGroup=(Zeno))
class ZENOLIVELINKRUNTIME_API UZenoPCGPointData : public UZenoPCGSpatialData
{
	GENERATED_BODY()

	// ~Begin UZenoPCGSpatialData interface
	virtual int32 GetDimension() const override { return 0; }
	virtual bool SamplePoint(const FTransform& InTransform, const FBox& Bounds, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const override;
	// ~End UZenoPCGSpatialData interface

	TArray<FZenoPCGPoint>& GetPoints();

protected:
	UPROPERTY()
	TArray<FZenoPCGPoint> Points;
};
