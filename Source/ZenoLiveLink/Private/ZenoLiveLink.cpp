#include "ZenoLiveLink.h"

#include "EditorModeManager.h"
#include "LevelEditor.h"
#include "ZenoBridge.h"
#include "Editor.h"
#include "Command/FZenoLandscapeCommand.h"
#include "Modules/ModuleManager.h"
#include "UI/Landscape/LandscapeToolZenoBridge.h"
#include "LandscapeEditorModule.h"
#include "UI/Landscape/LandscapeFileFormatZeno.h"

#define LOCTEXT_NAMESPACE "FZenoLiveLinkModule"

void FZenoLiveLinkModule::StartupModule()
{
	ModuleInstance = this;
	
	FZenoLandscapeCommand::Register();
	PluginCommands = MakeShared<FUICommandList>();
	MapPluginActions();
	
	GLevelEditorModeTools().OnEditorModeIDChanged().AddRaw(this, &FZenoLiveLinkModule::OnEditorModeChanged);

	ILandscapeEditorModule& LandscapeEditorModule = FModuleManager::Get().GetModuleChecked<ILandscapeEditorModule>("LandscapeEditor");
	LandscapeEditorModule.RegisterHeightmapFileFormat(MakeShared<FLandscapeHeightmapFileFormatZeno_Virtual>());
}

void FZenoLiveLinkModule::ShutdownModule()
{
	FZenoLandscapeCommand::Unregister();
	PluginCommands.Reset();
	UZenoLandscapeTool::UnRegister();
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

void FZenoLiveLinkModule::MapPluginActions() const
{
	if (!PluginCommands.IsValid()) return;
}

void FZenoLiveLinkModule::OnEditorModeChanged(const FEditorModeID& InModeID, bool bIsEnteringMode)
{
	if (InModeID == "EM_Landscape")
	{
		if (bIsEnteringMode)
		{
			UZenoLandscapeTool::Register();
			LandscapeTool->InitWidget();
		}
		else
		{
			UZenoLandscapeTool::UnRegister();
		}
	}
}

IMPLEMENT_MODULE(FZenoLiveLinkModule, ZenoLiveLink)
#undef LOCTEXT_NAMESPACE
