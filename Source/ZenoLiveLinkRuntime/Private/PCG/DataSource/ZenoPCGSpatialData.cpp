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

void UZenoPCGSpatialDataWithPointCache::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	Super::GetResourceSizeEx(CumulativeResourceSize);
}
