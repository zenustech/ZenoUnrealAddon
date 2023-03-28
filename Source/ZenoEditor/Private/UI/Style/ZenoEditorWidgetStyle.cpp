// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Style/ZenoEditorWidgetStyle.h"

FZenoEditorWidgetStyle::FZenoEditorWidgetStyle()
{
}

FZenoEditorWidgetStyle::~FZenoEditorWidgetStyle()
{
}

const FName FZenoEditorWidgetStyle::TypeName(TEXT("FZenoEditorWidgetStyle"));

void FZenoEditorWidgetStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
}

const FZenoEditorWidgetStyle& FZenoEditorWidgetStyle::GetDefault()
{
	static FZenoEditorWidgetStyle sDefault;
	return sDefault;
}
