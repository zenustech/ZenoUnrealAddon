#include "ZenoEditor.h"

#include "ZenoEditorCommand.h"
#include "Factory/Actor/ZenoGraphAssetActorFactory.h"
#include "UI/DetailPanel/ZenoDetailPanelService.h"
#include "UI/Menu/ZenoEditorMenuExtender.h"

#define LOCTEXT_NAMESPACE "FZenoEditorModule"

void FZenoEditorModule::StartupModule()
{
	FZenoEditorCommand::Register();
	FZenoEditorMenuExtender::Get().Register();
	RegisterActorFactory();
	RegisterDetailPanelCustomization();
}

void FZenoEditorModule::ShutdownModule()
{
	UnregisterDetailPanelCustomization();
	FZenoEditorMenuExtender::Get().Unregister();
}

void FZenoEditorModule::RegisterActorFactory() const
{
#if WITH_EDITORONLY_DATA
	auto ZenoGraphActorFactory = NewObject<UZenoGraphAssetActorFactory>();
	GEditor->ActorFactories.Add(ZenoGraphActorFactory);
#endif
}

void FZenoEditorModule::RegisterDetailPanelCustomization()
{
	FZenoDetailPanelServiceManager::Get().Register();
}

void FZenoEditorModule::UnregisterDetailPanelCustomization()
{
	FZenoDetailPanelServiceManager::Get().Unregister();
}

IMPLEMENT_MODULE(FZenoEditorModule, ZenoEditor)

#undef LOCTEXT_NAMESPACE
