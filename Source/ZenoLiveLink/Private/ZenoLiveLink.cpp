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
#include "Command/ZenoTextureExportCommand.h"
#include "Interfaces/ITextureEditorModule.h"
#include "UI/Landscape/LandscapeFileFormatZeno.h"
#include "UI/Landscape/TextureExportHelper.h"
#include "UI/Landscape/ZenoLandscapeDetailCustomization.h"

#define LOCTEXT_NAMESPACE "FZenoLiveLinkModule"

void FZenoLiveLinkModule::StartupModule()
{
	ModuleInstance = this;
	
	FZenoLandscapeCommand::Register();
	FZenoTextureExportCommand::Register();
	MapPluginActions();

	LandscapeTool = TStrongObjectPtr { NewObject<UZenoLandscapeTool>() };
	
	GLevelEditorModeTools().OnEditorModeIDChanged().AddRaw(this, &FZenoLiveLinkModule::OnEditorModeChanged);

	// Register our heightmap format
	ILandscapeEditorModule& LandscapeEditorModule = FModuleManager::Get().GetModuleChecked<ILandscapeEditorModule>("LandscapeEditor");
	LandscapeEditorModule.RegisterHeightmapFileFormat(MakeShared<FLandscapeHeightmapFileFormatZeno_Virtual>());

	FZenoLandscapeDetailCustomization::Register();

	RegisterAssets();
}

void FZenoLiveLinkModule::ShutdownModule()
{
	FZenoTextureExportCommand::Unregister();
	FZenoLandscapeCommand::Unregister();
	PluginCommands.Reset();
	FZenoLandscapeDetailCustomization::UnRegister();
	UnRegisterAssets();

	// Reset module pointer
	ModuleInstance = nullptr;
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

void FZenoLiveLinkModule::MapPluginActions()
{
	if (!PluginCommands.IsValid())
	{
		PluginCommands = MakeShared<FUICommandList>();
	}
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
