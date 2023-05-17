#include "UI/Menu/ZenoEditorMenuExtender.h"

#include "LevelEditor.h"

FZenoEditorExtenderServiceBase::FZenoEditorExtenderServiceBase()
	: CommandList(MakeShared<FUICommandList>())
{
}

void FZenoEditorExtenderServiceBase::Register()
{
	MapAction();
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	MenuBarExtender = MakeShareable(new FExtender);
	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuBarExtender);
}

void FZenoEditorExtenderServiceBase::Unregister()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuBarExtender);
}

void FZenoEditorMenuExtender::Register()
{
	for (const auto Pair : Services)
	{
		if (Pair.Value != nullptr)
		{
			Pair.Value->Register();
		}
	}
}

void FZenoEditorMenuExtender::Unregister()
{
	for (const auto Pair : Services)
	{
		if (Pair.Value != nullptr)
		{
			Pair.Value->Unregister();
		}
	}
}

void FZenoEditorMenuExtender::Add(const FName& InName, IZenoEditorExtenderService& Service)
{
	Services.Add(InName, &Service);
}
