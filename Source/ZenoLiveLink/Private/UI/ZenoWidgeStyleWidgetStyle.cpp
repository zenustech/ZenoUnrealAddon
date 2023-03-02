// Fill out your copyright notice in the Description page of Project Settings.


#include "ZenoWidgeStyleWidgetStyle.h"


FZenoWidgetStyle::FZenoWidgetStyle()
{
}

FZenoWidgetStyle::~FZenoWidgetStyle()
{
}

const FName FZenoWidgetStyle::TypeName(TEXT("FZenoWidgeStyleStyle"));

const FZenoWidgetStyle& FZenoWidgetStyle::GetDefault()
{
	static FZenoWidgetStyle Default;
	return Default;
}

void FZenoWidgetStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
}
