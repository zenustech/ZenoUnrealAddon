#include "ZenoEditor.h"

#include "ZenoEditorCommand.h"
#include "Factory/Actor/ZenoGraphAssetActorFactory.h"
#include "UI/Menu/ZenoEditorMenuExtender.h"

#define LOCTEXT_NAMESPACE "FZenoEditorModule"

void FZenoEditorModule::StartupModule()
{
	FZenoEditorCommand::Register();
	FZenoEditorMenuExtender::Get().Register();
	RegisterActorFactory();
}

void FZenoEditorModule::ShutdownModule()
{
	FZenoEditorMenuExtender::Get().Unregister();
}

void FZenoEditorModule::RegisterActorFactory() const
{
#if WITH_EDITORONLY_DATA
	auto ZenoGraphActorFactory = NewObject<UZenoGraphAssetActorFactory>();
	GEditor->ActorFactories.Add(ZenoGraphActorFactory);
#endif
}

IMPLEMENT_MODULE(FZenoEditorModule, ZenoEditor)

#undef LOCTEXT_NAMESPACE
