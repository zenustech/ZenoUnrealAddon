#include "ZenoEditor.h"

#include "ZenoEditorCommand.h"
#include "UI/Menu/ZenoEditorMenuExtender.h"

#define LOCTEXT_NAMESPACE "FZenoEditorModule"

void FZenoEditorModule::StartupModule()
{
	FZenoEditorCommand::Register();
	FZenoEditorMenuExtender::Get().Register();
}

void FZenoEditorModule::ShutdownModule()
{
	FZenoEditorMenuExtender::Get().Unregister();
}

IMPLEMENT_MODULE(FZenoEditorModule, ZenoEditor)

#undef LOCTEXT_NAMESPACE
