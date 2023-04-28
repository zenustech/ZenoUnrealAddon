#include "UI/Menu/ZenoEmbedGraphEditorExtenderService.h"

#include "LevelEditor.h"
#include "ZenoEditorCommand.h"
#include "ZenoGraphAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Misc/FileHelper.h"

PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_START
#include "ThirdParty/msgpack.h"
#include "zeno/zeno.h"
#include "zeno/core/Graph.h"
#include "zeno/unreal/UnrealTool.h"
#include "zeno/unreal/Pair.h"
THIRD_PARTY_INCLUDES_END
PRAGMA_POP_PLATFORM_DEFAULT_PACKING

#define LOCTEXT_NAMESPACE "FZenoEmbedGraphEditorExtenderService"

FZenoEmbedGraphEditorExtenderService::FZenoEmbedGraphEditorExtenderService()
	: FZenoEditorExtenderServiceBase()
{
}

void FZenoEmbedGraphEditorExtenderService::Register()
{
	FZenoEditorExtenderServiceBase::Register();
	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::After, CommandList, FMenuBarExtensionDelegate::CreateRaw(this, &FZenoEmbedGraphEditorExtenderService::ExtendTopMenuBar));
}

void FZenoEmbedGraphEditorExtenderService::Unregister()
{
	FZenoEditorExtenderServiceBase::Unregister();
}

void FZenoEmbedGraphEditorExtenderService::ExtendTopMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddPullDownMenu(LOCTEXT("Zeno", "Zeno Graph"), LOCTEXT("ZenoTooltip", "Zeno Tools"), FNewMenuDelegate::CreateRaw(this, &FZenoEmbedGraphEditorExtenderService::ExtendEmbedPullDownMenu), ZenoGraphHookLabel, ZenoGraphHookLabel);
}

void FZenoEmbedGraphEditorExtenderService::ExtendEmbedPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().ImportZslFile);
}

void FZenoEmbedGraphEditorExtenderService::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FZenoEmbedGraphEditorExtenderService::Debug));
	CommandList->MapAction(FZenoEditorCommand::Get().ImportZslFile, FExecuteAction::CreateRaw(this, &FZenoEmbedGraphEditorExtenderService::ImportZslFile));
}

void FZenoEmbedGraphEditorExtenderService::Debug()
{
	TArray<FString> Files;
	UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files);
	if (Files.Num() == 0)
	{
		return;
	}
	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *Files[0]))
	{
		return;
	}
	Json = Json.Replace(TEXT("\n"), TEXT(""));
	
	auto Graph = zeno::getSession().createGraph();
	std::map<std::string, zeno::zany> Inputs, Outputs;
	Graph->loadGraph(TCHAR_TO_ANSI(*Json));
	auto JsonBuffer = zeno::GetGraphInputParams(Graph.get());
	std::error_code Err;
	auto Params = msgpack::unpack<zeno::unreal::SubnetNodeParamList>(reinterpret_cast<uint8*>(JsonBuffer.data), JsonBuffer.length - 1, Err);
	// auto Output = zeno::CallSubnetNode(Graph.get(), "fa438882-tower", 4, std::pair<const char*, zeno::zany>{ "TowerHeight", std::make_shared<zeno::NumericObject>(18) }, std::pair<const char*, zeno::zany>{ "TowerRadius", std::make_shared<zeno::NumericObject>(2) }, std::pair<const char*, zeno::zany>{ "TowerPillars", std::make_shared<zeno::NumericObject>(6) }, std::pair<const char*, zeno::zany>{ "TowerPillarHeight", std::make_shared<zeno::NumericObject>(3) });
	// auto Output = zeno::CallSubnetNode_Mesh(Graph.get(), "fa438882-tower", 4, std::pair<const char*, zeno::zany>{ "TowerHeight", std::make_shared<zeno::NumericObject>(18) }, std::pair<const char*, zeno::zany>{ "TowerRadius", std::make_shared<zeno::NumericObject>(2) }, std::pair<const char*, zeno::zany>{ "TowerPillars", std::make_shared<zeno::NumericObject>(6) }, std::pair<const char*, zeno::zany>{ "TowerPillarHeight", std::make_shared<zeno::NumericObject>(3) });
	// auto Data = msgpack::unpack<zeno::unreal::Mesh>(reinterpret_cast<uint8*>(Output.data), Output.length - 1, Err);
	UE_LOG(LogTemp, Error, TEXT("Out Num: %llu"), Params.params.size());
}

void FZenoEmbedGraphEditorExtenderService::ImportZslFile()
{
	TArray<FString> Files;
	if (UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files, "Open File", "", "", "ZSL File|*.json") && Files.Num() > 0)
	{
		for (const FString& File : Files)
		{
			FString JsonData;
			if (!FFileHelper::LoadFileToString(JsonData, *File)) { continue; }
			if (!zeno::IsValidZSL(TCHAR_TO_ANSI(*JsonData)))
			{
				// TODO [darc] : pop warning window :
				UE_LOG(LogTemp, Error, TEXT("File '%ls' is not a valid zsl file."), *File);
				continue;
			}
			if (const FString ContentPath = UZenoCommonBlueprintLibrary::OpenContentPicker(); !ContentPath.IsEmpty())
			{
				const FString SavePackageName = FPackageName::ObjectPathToPackageName(ContentPath);
				const FString SavePackagePath = FPaths::GetPath(SavePackageName);
				const FString SaveAssetName = FPaths::GetBaseFilename(SavePackageName);
				UPackage* Package = CreatePackage(*SavePackageName);
				UZenoGraphAsset* GraphAsset = NewObject<UZenoGraphAsset>(Package, UZenoGraphAsset::StaticClass(), FName(SaveAssetName), RF_Public | RF_Standalone);
				if (IsValid(GraphAsset))
				{
					GraphAsset->PreEditChange(nullptr);
					GraphAsset->ZenoActionRecordExportedData = MoveTempIfPossible(JsonData);
					GraphAsset->ReimportPath = File;
					GraphAsset->PostEditChange();
					FAssetRegistryModule::AssetCreated(GraphAsset);
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
