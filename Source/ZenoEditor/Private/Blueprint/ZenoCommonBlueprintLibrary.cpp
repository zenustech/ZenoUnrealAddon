// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ZenoCommonBlueprintLibrary.h"

#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "IContentBrowserSingleton.h"
#include "IDesktopPlatform.h"

#define LOCTEXT_NAMESPACE "UZenoCommonBlueprintLibrary"

bool UZenoCommonBlueprintLibrary::OpenSystemFilePicker(TArray<FString>& OutFiles, const FString& Title /** = "Open File" */, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, const int32 Flags)
{
	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		 return DesktopPlatform->OpenFileDialog(nullptr, Title, DefaultPath, DefaultFile, FileTypes, Flags, OutFiles);
	}
	return false;
}

FString UZenoCommonBlueprintLibrary::OpenContentPicker(const FString& AssetName, const FString& PackagePath)
{
	FSaveAssetDialogConfig SaveAssetDialogConfig;
	SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("SelectPackage", "Select Package");
	SaveAssetDialogConfig.DefaultPath = PackagePath;
	SaveAssetDialogConfig.DefaultAssetName = AssetName;
	SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;

	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
	return SaveObjectPath;
}

#undef LOCTEXT_NAMESPACE
