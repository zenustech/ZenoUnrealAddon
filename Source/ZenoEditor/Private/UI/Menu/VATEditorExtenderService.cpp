#include "UI/Menu/VATEditorExtenderService.h"

#include "AssetToolsModule.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "LevelEditor.h"
#include "ProceduralMeshComponent.h"
#include "RawMesh.h"
#include "StaticMeshAttributes.h"
#include "ZenoEditorCommand.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Blueprint/ZenoEditorSettings.h"
#include "Importer/VAT/VATTypes.h"
#include "Importer/VAT/VATUtility.h"
#include "Importer/Wavefront/ZenoObjLoader.h"
#include "Materials/MaterialInstance.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "FVATEditorExtenderService"

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
	const FName ZenoHook = "MENU_Zeno";
	Builder.AddPullDownMenu(LOCTEXT("Zeno", "Zeno"), LOCTEXT("ZenoTooltip", "Zeno Tools"), FNewMenuDelegate::CreateRaw(this, &FVATEditorExtenderService::ExtendVATPullDownMenu), ZenoHook, ZenoHook);
}

void FVATEditorExtenderService::ExtendVATPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().ImportWavefrontMesh);
	Builder.AddMenuEntry(FZenoEditorCommand::Get().Debug);
}

void FVATEditorExtenderService::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FVATEditorExtenderService::Debug));
	CommandList->MapAction(FZenoEditorCommand::Get().ImportWavefrontMesh, FExecuteAction::CreateRaw(this, &FVATEditorExtenderService::ImportWavefrontObjFile));
}

FVATEditorExtenderService& FVATEditorExtenderService::Get()
{
	static FVATEditorExtenderService Service;
	return Service;
}

void FVATEditorExtenderService::Debug()
{
	TArray<FString> Files;
	if (UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files) && Files.Num() > 0)
	{
		FVATInfo Info;
		UVATUtility::ParseBinaryInfo(Files[0], Info);
	}
}

void FVATEditorExtenderService::ImportWavefrontObjFile()
{
	const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>(
    			"AssetTools");
	TArray<FString> Files;
	if (UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files), "Open File", "", "", ".bin")
	{
		TArray<FString> Arr;
		FFileHelper::LoadFileToStringArray(Arr, *Files[0]);
		const FWavefrontFileParser Parser{Arr};
		EWavefrontParseError Err;
		TSharedPtr<FRawMesh> RawMesh = Parser.Parse(Err);
		if (Err == EWavefrontParseError::Success)
		{
			UStaticMesh* StaticMesh = NewObject<UStaticMesh>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UStaticMesh::StaticClass()), RF_Public | RF_Standalone);
			StaticMesh->ImportVersion = LastVersion;
			StaticMesh->PreEditChange(nullptr);
			FStaticMeshSourceModel& SourceModel = StaticMesh->AddSourceModel();
			{
				StaticMesh->NaniteSettings.bEnabled = false;
				SourceModel.BuildSettings.bRecomputeNormals = false;
				SourceModel.BuildSettings.bRecomputeTangents = true;
				SourceModel.BuildSettings.bRemoveDegenerates = false;
				SourceModel.BuildSettings.bComputeWeightedNormals = false;
				SourceModel.BuildSettings.bUseMikkTSpace = false;
				SourceModel.BuildSettings.bUseFullPrecisionUVs = true;
				SourceModel.BuildSettings.bUseHighPrecisionTangentBasis = true;
				SourceModel.SaveRawMesh(*RawMesh);
			}
			StaticMesh = Cast<UStaticMesh>(AssetToolsModule.Get().DuplicateAssetWithDialog(MakeUniqueObjectName(GetTransientPackage(), UStaticMesh::StaticClass()).ToString(), "/GAME", StaticMesh));
			FName MaterialSlotName = NAME_None;
			FStaticMaterial StaticMaterial;
			if (UZenoEditorSettings::Get()->bAutoCreateBasicVatMaterialInstanceConstant)
			{
				UMaterialInstance* NewMI = UZenoEditorSettings::CreateBasicVATMaterialInstance(FString::Printf(TEXT("Mat_%ls_Inst"), *StaticMesh->GetName()), StaticMesh->GetFullName());
				MaterialSlotName = StaticMesh->AddMaterial(NewMI);
				StaticMaterial.MaterialInterface = NewMI;
			}
			StaticMaterial.MaterialSlotName = MaterialSlotName;
			StaticMesh->GetStaticMaterials().Add(StaticMaterial);
			StaticMesh->PostEditChange();
			StaticMesh->Build(false);
			FAssetRegistryModule::AssetCreated(StaticMesh);
			if (IsValid(StaticMesh->GetPackage()))
			{
				auto _ = StaticMesh->GetPackage()->MarkPackageDirty();
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("VAT", FVATEditorExtenderService);
