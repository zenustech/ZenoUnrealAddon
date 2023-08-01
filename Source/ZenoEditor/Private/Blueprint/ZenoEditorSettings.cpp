// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ZenoEditorSettings.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"

UZenoEditorSettings::UZenoEditorSettings()
	: bAutoCreateBasicVatMaterialInstanceConstant(true)
	, DefaultBasicVATMaterial(FSoftObjectPath("/ZenoEngine/Material/Mat_PositionAndNormalVAT.Mat_PositionAndNormalVAT"))
	, VATImportScale(1.0f)
	, DefaultImportedLandscapeMaterialParent(FSoftObjectPath("/ZenoEngine/Material/Mat_DefaultLandscapeMaterial.Mat_DefaultLandscapeMaterial"))
{
}

const UZenoEditorSettings* UZenoEditorSettings::Get()
{
	return GetDefault<UZenoEditorSettings>();
}

UMaterialInstance* UZenoEditorSettings::CreateBasicVATMaterialInstance(const FString& InName, const FString& PackagePath)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UZenoEditorSettings::CreateBasicVATMaterialInstance);
	const FString SavePath = UZenoCommonBlueprintLibrary::OpenContentPicker(InName, PackagePath);

	if (!SavePath.IsEmpty())
	{
		const FString SavePackageName = FPackageName::ObjectPathToPackageName(SavePath);
		const FString SavePackagePath = FPaths::GetPath(SavePackageName);
		const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);

		UMaterialInterface* ParentMaterial = Get()->DefaultBasicVATMaterial.LoadSynchronous();
		UPackage* Package = CreatePackage(*SavePackageName);
		UMaterialInstance* NewMaterial = NewObject<UMaterialInstance>(Package, UMaterialInstanceConstant::StaticClass(), FName(SaveAssetName), RF_Standalone | RF_Public);
		NewMaterial->PreEditChange(nullptr);
		NewMaterial->Parent = ParentMaterial;
		NewMaterial->PostEditChange();
		FAssetRegistryModule::AssetCreated(NewMaterial);
		if (IsValid(Package))
		{
			auto _ = Package->MarkPackageDirty();
		}
		return NewMaterial;
	}

	return nullptr;
}
