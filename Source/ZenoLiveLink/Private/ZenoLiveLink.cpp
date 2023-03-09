#include "ZenoLiveLink.h"

#include "AssetToolsModule.h"
#include "EditorModeManager.h"
#include "LevelEditor.h"
#include "ZenoBridge.h"
#include "Editor.h"
#include "EditorModes.h"
#include "Command/FZenoLandscapeCommand.h"
#include "Modules/ModuleManager.h"
#include "UI/Landscape/LandscapeToolZenoBridge.h"
#include "LandscapeEditorModule.h"
#include "Asset/ZenoAssetLandscapeActorFactory.h"
#include "Asset/ZenoBridgeAssetFactory.h"
#include "UI/Landscape/LandscapeFileFormatZeno.h"
#include "UI/Landscape/ZenoLandscapeDetailCustomization.h"

#define LOCTEXT_NAMESPACE "FZenoLiveLinkModule"

void FZenoLiveLinkModule::StartupModule()
{
	ModuleInstance = this;
	
	FZenoLandscapeCommand::Register();
	PluginCommands = MakeShared<FUICommandList>();
	MapPluginActions();

	LandscapeTool = TStrongObjectPtr { NewObject<UZenoLandscapeTool>() };
	
	GLevelEditorModeTools().OnEditorModeIDChanged().AddRaw(this, &FZenoLiveLinkModule::OnEditorModeChanged);

	ILandscapeEditorModule& LandscapeEditorModule = FModuleManager::Get().GetModuleChecked<ILandscapeEditorModule>("LandscapeEditor");
	LandscapeEditorModule.RegisterHeightmapFileFormat(MakeShared<FLandscapeHeightmapFileFormatZeno_Virtual>());

	FZenoLandscapeDetailCustomization::Register();

	RegisterAssets();
}

void FZenoLiveLinkModule::ShutdownModule()
{
	FZenoLandscapeCommand::Unregister();
	PluginCommands.Reset();
	ModuleInstance = nullptr;
	FZenoLandscapeDetailCustomization::UnRegister();
	UnRegisterAssets();
}

void FZenoLiveLinkModule::Tick(float DeltaTime)
{
}

bool FZenoLiveLinkModule::IsTickable() const
{
	return false;
}

TStatId FZenoLiveLinkModule::GetStatId() const
{
	return {};
}

TWeakObjectPtr<UZenoLandscapeTool> FZenoLiveLinkModule::GetLandscapeTool() const
{
	return TWeakObjectPtr<UZenoLandscapeTool>(LandscapeTool.Get());
}

FZenoLiveLinkModule* FZenoLiveLinkModule::GetInstance()
{
	return ModuleInstance;
}

void FZenoLiveLinkModule::MapPluginActions() const
{
	if (!PluginCommands.IsValid()) return;
}

void FZenoLiveLinkModule::RegisterAssets()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	const TSharedRef<IAssetTypeActions> ZenoBridgeAssetActions = MakeShared<FZenoBridgeAssetActions>();
	EncounteredAssets.Add(ZenoBridgeAssetActions);
	AssetTools.RegisterAssetTypeActions(ZenoBridgeAssetActions);

	auto LandscapeActorFactory = NewObject<UZenoAssetLandscapeActorFactory>();
	GEditor->ActorFactories.Add(LandscapeActorFactory);
}

void FZenoLiveLinkModule::UnRegisterAssets()
{
	if (const FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools"); nullptr != AssetToolsModule)
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();

		for (auto Action : EncounteredAssets)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
}

void FZenoLiveLinkModule::OnEditorModeChanged(const FEditorModeID& InModeID, bool bIsEnteringMode)
{
	if (InModeID == FBuiltinEditorModes::EM_Landscape)
	{
		if (bIsEnteringMode)
		{
			LandscapeTool->InvokeUI();
		}
		else
		{
			LandscapeTool->RequestToClose();
		}
	}
}

IMPLEMENT_MODULE(FZenoLiveLinkModule, ZenoLiveLink)
#undef LOCTEXT_NAMESPACE
