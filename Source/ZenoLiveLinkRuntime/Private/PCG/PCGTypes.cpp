// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/PCGTypes.h"

FZenoPCGPoint::FZenoPCGPoint(const FTransform& InTransform, float InDensity, int32 InSeed)
	: Transform(InTransform)
	, Density(InDensity)
	, Seed(InSeed)
{
}
