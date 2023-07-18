// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/ZenoLandscapeAssetFactory.h"

#define LOCTEXT_NAMESPACE "ZenoLandscapeAssetFactory"

UZenoLandscapeAssetFactory::UZenoLandscapeAssetFactory(const FObjectInitializer& ObjectInitializer)
{
	Formats.Add("zlasset");
	bText = false;
	bEditorImport = true;
}

UObject* UZenoLandscapeAssetFactory::FactoryCreateBinary(UClass* InClass, UObject* InParent, FName InName,
                                                         EObjectFlags Flags, UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
                                                         FFeedbackContext* Warn)
{
	return Super::FactoryCreateBinary(InClass, InParent, InName, Flags, Context, Type, Buffer, BufferEnd, Warn);
}

bool UZenoLandscapeAssetFactory::CanCreateNew() const
{
	return true;
}

FText UZenoLandscapeAssetFactory::GetDisplayName() const
{
	return LOCTEXT("Name", "Zeno Landscape Asset");
}

#undef LOCTEXT_NAMESPACE
