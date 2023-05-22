// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/DataSource/ZenoPCGPointData.h"

bool UZenoPCGPointData::SamplePoint(const FTransform& InTransform, const FBox& Bounds, FZenoPCGPoint& OutPoint,
	UZenoMetadata* OutMetadata) const
{
	// TODO [darc]
	return false;
}

TArray<FZenoPCGPoint>& UZenoPCGPointData::GetPoints()
{
	return Points;
}
