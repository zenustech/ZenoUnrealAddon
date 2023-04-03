#include "UI/Menu/ZenoEmbedGraphEditorExtenderService.h"

#include "LevelEditor.h"
#include "ZenoEditorCommand.h"

PRAGMA_PUSH_PLATFORM_DEFAULT_PACKING
THIRD_PARTY_INCLUDES_START
#include "zeno/core/Session.h"
#include "zeno/core/Graph.h"
#include "zeno/core/INode.h"
#include "zeno/DictObject.h"
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
	auto Graph = zeno::getSession().createGraph();
	std::map<std::string, zeno::zany> Inputs, Outputs;
	Outputs = Graph->callTempNode("EmptyDict", std::move(Inputs));
	UE_LOG(LogTemp, Error, TEXT("Num: %llu"), Outputs.size());
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("Embed", FZenoEmbedGraphEditorExtenderService);
