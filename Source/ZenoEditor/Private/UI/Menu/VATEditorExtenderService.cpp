#include "UI/Menu/VATEditorExtenderService.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "LevelEditor.h"
#include "ZenoEditorCommand.h"
#include "VAT/VATTypes.h"
#include "VAT/VATUtility.h"


bool OpenFileDialog(TArray<FString>& OutFiles)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		 return DesktopPlatform->OpenFileDialog(nullptr, "Test", "", "", "", 0, OutFiles);
	}
	return false;
}

FVATEditorExtenderService::FVATEditorExtenderService()
	: CommandList(MakeShared<FUICommandList>())
{
}

void FVATEditorExtenderService::Register()
{
	MapAction();
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	MenuBarExtender = MakeShareable(new FExtender);

	MenuBarExtender->AddMenuBarExtension("Help", EExtensionHook::After, CommandList, FMenuBarExtensionDelegate::CreateRaw(this, &FVATEditorExtenderService::ExtendMenuBar));

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuBarExtender);
}

void FVATEditorExtenderService::Unregister()
{
}

void FVATEditorExtenderService::ExtendMenuBar(FMenuBarBuilder& Builder)
{
	Builder.AddMenuEntry(FZenoEditorCommand::Get().Debug);
}

void FVATEditorExtenderService::MapAction()
{
	CommandList->MapAction(FZenoEditorCommand::Get().Debug, FExecuteAction::CreateRaw(this, &FVATEditorExtenderService::Debug));
}

FVATEditorExtenderService& FVATEditorExtenderService::Get()
{
	static FVATEditorExtenderService Service;
	return Service;
}

void FVATEditorExtenderService::Debug()
{
	TArray<FString> Files;
	if (OpenFileDialog(Files))
	{
		FVATInfo Info;
		if (UVATUtility::ParseBinaryInfo(Files[0], Info))
		{
			UE_LOG(LogTemp, Error, TEXT("%hs"), reinterpret_cast<const char*>(Info.Magic.data()));
		}
	}
}

REGISTER_EDITOR_EXTENDER_SERVICE("VAT", FVATEditorExtenderService);
