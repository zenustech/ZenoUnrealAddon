// Fill out your copyright notice in the Description page of Project Settings.


#include "Factory/ZenoLandscapeAssetFactory.h"

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
