// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoPCGSpatialData.h"
#include "ZenoPCGSurfaceData.generated.h"

UCLASS(Abstract, BlueprintType, ClassGroup = (Zeno))
class ZENOLIVELINKRUNTIME_API UZenoPCGSurfaceData : public UZenoPCGSpatialDataWithPointCache
{
	GENERATED_BODY()

public:
	// ~Begin UZenoPCGData interface
	virtual EZenoPCGDataType GetDataType() const override { return EZenoPCGDataType::Surface; }
	// ~End UZenoPCGData interface

	//~Begin UZenoPCGSpatialData interface
	virtual int GetDimension() const override { return 2; }
	//~End UZenoPCGSpatialData interface

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = SpatialData)
	FTransform Transform;
};
