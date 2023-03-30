#include "UI/Menu/VATEditorExtenderService.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "LevelEditor.h"
#include "ProceduralMeshComponent.h"
#include "RawMesh.h"
#include "StaticMeshAttributes.h"
#include "ZenoEditorCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Importer/VAT/VATTypes.h"
#include "Importer/VAT/VATUtility.h"
#include "Importer/Wavefront/ZenoObjLoader.h"
#include "Misc/FileHelper.h"


bool OpenFileDialog(TArray<FString>& OutFiles)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		 return DesktopPlatform->OpenFileDialog(nullptr, "Test", "", "", "", 0, OutFiles);
	}
	return false;
}

FVATEditorExtenderService::FVATEditorExtenderService()
	: CommandList(MakeShared<FUICommandList>())
{
}

void FVATEditorExtenderService::Register()
{
	MapAction();
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	MenuBarExtender = MakeShareable(new FExtender);

	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::After, CommandList, FMenuBarExtensionDelegate::CreateRaw(this, &FVATEditorExtenderService::ExtendMenuBar));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuBarExtender);
}

void FVATEditorExtenderService::Unregister()
{
}

void FVATEditorExtenderService::ExtendMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().Debug);
}

void FVATEditorExtenderService::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FVATEditorExtenderService::Debug));
}

FVATEditorExtenderService& FVATEditorExtenderService::Get()
{
	static FVATEditorExtenderService Service;
	return Service;
}

void FVATEditorExtenderService::Debug()
{
	TArray<FString> Files;
	if (OpenFileDialog(Files))
	{
		TArray<FString> Arr;
		FFileHelper::LoadFileToStringArray(Arr, *Files[0]);
		const FWavefrontFileParser Parser{Arr};
		EWavefrontParseError Err;
		TSharedPtr<FRawMesh> RawMesh = Parser.Parse(Err);
		if (Err == EWavefrontParseError::Success)
		{
			UPackage* MeshPackage = CreatePackage(TEXT("/Game/TestStaticMesh"));
			UStaticMesh* StaticMesh = NewObject<UStaticMesh>(MeshPackage, MakeUniqueObjectName(MeshPackage, UStaticMesh::StaticClass()), RF_Public | RF_Standalone);
			StaticMesh->ImportVersion = LastVersion;
			StaticMesh->PreEditChange(nullptr);
			{
				StaticMesh->NaniteSettings.bEnabled = false;
				FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
				SourceModel.BuildSettings.bRecomputeNormals = false;
				SourceModel.BuildSettings.bRecomputeTangents = false;
				SourceModel.BuildSettings.bRemoveDegenerates = false;
				SourceModel.BuildSettings.bComputeWeightedNormals = false;
				SourceModel.BuildSettings.BuildScale3D = {10.0, 10.0, 10.0};
				SourceModel.BuildSettings.bUseFullPrecisionUVs = true;
				SourceModel.SaveRawMesh(*RawMesh);
			}
			StaticMesh->PostEditChange();
			StaticMesh->Build(false);
			FAssetRegistryModule::AssetCreated(StaticMesh);
		}
	}
}

REGISTER_EDITOR_EXTENDER_SERVICE("VAT", FVATEditorExtenderService);
