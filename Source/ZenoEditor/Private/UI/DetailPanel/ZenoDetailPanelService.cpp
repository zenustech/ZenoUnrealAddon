#include "ZenoDetailPanelService.h"

FZenoDetailPanelServiceManager& FZenoDetailPanelServiceManager::Get()
{
	static FZenoDetailPanelServiceManager Manager;
	return Manager;
}

void FZenoDetailPanelServiceManager::Register()
{
	FZenoDetailPanelServiceManager& Manager = Get();
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	for (const auto& Pair : Manager.DetailCustomizations)
	{
		PropertyEditorModule.RegisterCustomClassLayout(Pair.Key, Pair.Value);
	}
}

void FZenoDetailPanelServiceManager::Unregister()
{
	FZenoDetailPanelServiceManager& Manager = Get();
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	for (const auto& Pair : Manager.DetailCustomizations)
	{
		PropertyEditorModule.UnregisterCustomClassLayout(Pair.Key);
	}
}

void FZenoDetailPanelServiceManager::RegisterDetailCustomization(const FName& InName,
                                                                 const FOnGetDetailCustomizationInstance& DetailCustomization)
{
	DetailCustomizations.Add(InName, DetailCustomization);
}
