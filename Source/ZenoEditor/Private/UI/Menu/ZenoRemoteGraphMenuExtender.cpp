#include "UI/Menu/ZenoRemoteGraphMenuExtender.h"

#include "ZenoEditorCommand.h"
#include "Blueprint/ZenoCommonBlueprintLibrary.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Misc/FileHelper.h"

#define LOCTEXT_NAMESPACE "FZenoRemoteGraphMenuExtender"

void FZenoRemoteGraphMenuExtender::Register()
{
	FZenoEditorExtenderServiceBase::Register();
	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::After, CommandList, FMenuBarExtensionDelegate::CreateRaw(this, &FZenoRemoteGraphMenuExtender::ExtendTopMenuBar));
}

void FZenoRemoteGraphMenuExtender::Unregister()
{
	FZenoEditorExtenderServiceBase::Unregister();
}

void FZenoRemoteGraphMenuExtender::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FZenoRemoteGraphMenuExtender::Debug));
	CommandList->MapAction(FZenoEditorCommand::Get().ImportZslFile, FExecuteAction::CreateRaw(this, &FZenoRemoteGraphMenuExtender::ImportZslFile));
}

void FZenoRemoteGraphMenuExtender::ExtendTopMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddPullDownMenu(LOCTEXT("ZenoGraph", "Zeno Graph"), LOCTEXT("ZenoGraphTooltips", "Graph actions"), FNewMenuDelegate::CreateRaw(this, &FZenoRemoteGraphMenuExtender::ExtendEmbedPullDownMenu));
}

void FZenoRemoteGraphMenuExtender::ExtendEmbedPullDownMenu(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().ZenoGraphHeader);
	Builder.AddSeparator();
	Builder.AddMenuEntry(FZenoEditorCommand::Get().Debug);
}

void FZenoRemoteGraphMenuExtender::Debug()
{
}

void FZenoRemoteGraphMenuExtender::ImportZslFile()
{
	UZenoLiveLinkClientSubsystem* LiveLinkSubsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	UZenoLiveLinkSession* Session = LiveLinkSubsystem->GetSessionFallback();
	if (!IsValid(Session)) {return;}

	// Let user pick some files, blocking
	TArray<FString> Files;
	if (UZenoCommonBlueprintLibrary::OpenSystemFilePicker(Files, "Open File", "", "", "ZSL File|*.json") && Files.Num() > 0)
	{
		for (const FString& File : Files)
		{
			FString JsonData;
			// Try to load file into JsonData
			if (!FFileHelper::LoadFileToString(JsonData, *File)) { continue; }
		}
	}
}

#undef LOCTEXT_NAMESPACE

REGISTER_EDITOR_EXTENDER_SERVICE("RemoteGraph", FZenoRemoteGraphMenuExtender);
