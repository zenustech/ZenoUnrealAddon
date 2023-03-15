// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Landscape/ZenoLandscapeEditorObject.h"

UZenoLandscapeEditorObject::UZenoLandscapeEditorObject()
{
	LayerVisual_VisualData = Cast<UZenoLandscape_VisualData>(UZenoLandscape_VisualData::StaticClass()->ClassDefaultObject);
}
