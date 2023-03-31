#include "UI/Menu/ZenoEmbedGraphEditorExtenderService.h"

#include "LevelEditor.h"
#include "ZenoEditorCommand.h"

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
	UE_LOG(LogTemp, Error, TEXT("123"));
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("Embed", FZenoEmbedGraphEditorExtenderService);
