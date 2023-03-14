#include "TextureExportHelper.h"

#include "Interfaces/ITextureEditorModule.h"

TSharedRef<FUICommandList> FTextureExportHelper::GetStaticCommandList()
{
	static TSharedRef<FUICommandList> SCommandList = MakeShared<FUICommandList>();
	static std::atomic<bool> bHasMap = false;

	// Map command into list if not mapped
	if (bool bFlag = false; bHasMap.compare_exchange_strong(bFlag, true))
	{
	}

	return SCommandList;
}

TSharedRef<FExtender> FTextureExportHelper::MakeExtender()
{
	const TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension(FName("Asset"), EExtensionHook::Before, GetStaticCommandList(), FToolBarExtensionDelegate::CreateStatic(&FTextureExportHelper::BuildExportToolbar));

	return Extender;
}

void FTextureExportHelper::BuildExportToolbar(FToolBarBuilder& Builder)
{
	const FName TextureExportSection("TextureExport");
	Builder.BeginSection(TextureExportSection);

	Builder.EndSection();
}

void FTextureExportHelper::Register()
{
	
	ITextureEditorModule& TextureEditorModule = FModuleManager::LoadModuleChecked<ITextureEditorModule>("TextureEditor");
	const TSharedPtr<FExtensibilityManager> ToolbarExtensionManager = TextureEditorModule.GetToolBarExtensibilityManager();
	if (ToolbarExtensionManager.IsValid())
	{
		ToolbarExtensionManager->AddExtender(CurrentExtender);
	}
}

void FTextureExportHelper::Unregister()
{
	ITextureEditorModule& TextureEditorModule = FModuleManager::LoadModuleChecked<ITextureEditorModule>("TextureEditor");
	const TSharedPtr<FExtensibilityManager> ToolbarExtensionManager = TextureEditorModule.GetToolBarExtensibilityManager();
	if (ToolbarExtensionManager.IsValid())
	{
		ToolbarExtensionManager->RemoveExtender(CurrentExtender);
	}
}
