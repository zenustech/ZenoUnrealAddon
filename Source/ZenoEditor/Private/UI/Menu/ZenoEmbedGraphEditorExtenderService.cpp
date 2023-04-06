#include "UI/Menu/ZenoEmbedGraphEditorExtenderService.h"

#include "LevelEditor.h"
#include "ZenoEditorCommand.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Misc/FileHelper.h"

PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_START
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

FZenoEmbedGraphEditorExtenderService& FZenoEmbedGraphEditorExtenderService::Get()
{
	static FZenoEmbedGraphEditorExtenderService Service;
	return Service;
}

void FZenoEmbedGraphEditorExtenderService::ExtendTopMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddPullDownMenu(LOCTEXT("Zeno", "Zeno"), LOCTEXT("ZenoTooltip", "Zeno Tools"), FNewMenuDelegate::CreateRaw(this, &FZenoEmbedGraphEditorExtenderService::ExtendEmbedPullDownMenu), ZenoHookLabel, ZenoHookLabel);
}

void FZenoEmbedGraphEditorExtenderService::ExtendEmbedPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().Debug);
}

void FZenoEmbedGraphEditorExtenderService::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FZenoEmbedGraphEditorExtenderService::Debug));
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
	auto Result = zeno::CallTempNode(Graph.get(), "EmptyDict", 1, std::pair<const char*, zeno::zany>("123", {}));
	UE_LOG(LogTemp, Error, TEXT("Res Num: %llu"), Result.size());
	Graph->loadGraph(TCHAR_TO_ANSI(*Json));
	auto Output = zeno::CallSubnetNode(Graph.get(), "fa438882-tower", 4, std::pair<const char*, zeno::zany>{ "TowerHeight", std::make_shared<zeno::NumericObject>(18) }, std::pair<const char*, zeno::zany>{ "TowerRadius", std::make_shared<zeno::NumericObject>(2) }, std::pair<const char*, zeno::zany>{ "TowerPillars", std::make_shared<zeno::NumericObject>(6) }, std::pair<const char*, zeno::zany>{ "TowerPillarHeight", std::make_shared<zeno::NumericObject>(3) });
	UE_LOG(LogTemp, Error, TEXT("Out Num: %llu"), Output.size());
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("Embed", FZenoEmbedGraphEditorExtenderService);
