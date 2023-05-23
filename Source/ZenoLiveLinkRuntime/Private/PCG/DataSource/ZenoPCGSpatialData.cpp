// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/DataSource/ZenoPCGSpatialData.h"

#include "PCG/ZenoMetadata.h"

UZenoPCGSpatialData::UZenoPCGSpatialData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZenoPCGSpatialData::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	Super::GetResourceSizeEx(CumulativeResourceSize);

	if (Metadata)
	{
		Metadata->GetResourceSizeEx(CumulativeResourceSize);
	}
}

bool UZenoPCGSpatialData::ProjectPoint(const FTransform& InTransform, const FBox& InBounds,
	const FZenoPCGProjectionParams& InParams, FZenoPCGPoint& OutPoint, UZenoMetadata* OutMetadata) const
{
	const bool bResult = SamplePoint(InTransform, InBounds, OutPoint, OutMetadata);

	if (!InParams.bProjectPositions)
	{
		OutPoint.Transform.SetLocation(InTransform.GetLocation());
	}

	if (!InParams.bProjectRotations)
	{
		OutPoint.Transform.SetRotation(InTransform.GetRotation());
	}

	if (!InParams.bProjectScales)
	{
		OutPoint.Transform.SetScale3D(InTransform.GetScale3D());
	}

	return bResult;
}

void UZenoPCGSpatialDataWithPointCache::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	Super::GetResourceSizeEx(CumulativeResourceSize);
}
