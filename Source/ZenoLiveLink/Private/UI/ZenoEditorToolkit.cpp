// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZenoEditorToolkit.h"

#define LOCTEXT_NAMESPACE "UZenoEditorToolkit"

UZenoEditorToolkit::UZenoEditorToolkit(const FObjectInitializer& ObjectInitializer)
	: UICommandList(new FUICommandList)
{
	DisplayName = LOCTEXT("ToolkitDisplayName", "Zeno Toolkit");
	MenuType = ETabSpawnerMenuType::Hidden;
	TabId = NAME_None;
}

void UZenoEditorToolkit::Init()
{
}

void UZenoEditorToolkit::Register()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabId,
		FOnSpawnTab::CreateUObject(this, &UZenoEditorToolkit::GetDockTab),
		FCanSpawnTab::CreateWeakLambda(this, [this](const FSpawnTabArgs& Args) { return CanBeCreate(Args); })
	)
	.SetDisplayName(LOCTEXT("ZenoLandscapeTool", "Zeno Landscape Tool"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);
	Init();
	// UICommandList->MapAction(, );
}

void UZenoEditorToolkit::UnRegister()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabId);
}

bool UZenoEditorToolkit::CanBeCreate(const FSpawnTabArgs& Args)
{
	return false;
}

void UZenoEditorToolkit::InvokeUI()
{
	Register();
	FGlobalTabmanager::Get()->TryInvokeTab(TabId);
}

void UZenoEditorToolkit::MakeDockTab(FName InMode)
{
	if (!Slate_ToolkitDockTab.IsValid()) return;

	const TSharedRef<SVerticalBox> TabBox = SNew(SVerticalBox);
	
	FVerticalToolBarBuilder CatalogBuilder { UICommandList, FMultiBoxCustomization::None };
	for (auto Catalog : ModeInfos)
	{
		const FName Name {Catalog.Key};
		UICommandList->MapAction(Catalog.Value, FUIAction { FExecuteAction::CreateUObject(this, &UZenoEditorToolkit::OnChangeMode, FName(Name)), FCanExecuteAction::CreateUObject(this, &UZenoEditorToolkit::IsModeEnabled, FName(Name)), FIsActionChecked::CreateUObject(this, &UZenoEditorToolkit::IsModeActive, FName(Name)) });
		CatalogBuilder.AddToolbarStackButton(Catalog.Value);
	}
	TSet<FName> Keys;
	ModeInfos.GetKeys(Keys);
	if (CurrentMode == NAME_None && Keys.Num() > 0)
	{
		CurrentMode = *Keys.begin();
	}
	TabBox->AddSlot().AutoHeight() [ CatalogBuilder.MakeWidget(nullptr, 150) ];

	FVerticalToolBarBuilder Builder { UICommandList, FMultiBoxCustomization::None };
	if (ToolPaletteBuilders.Contains(CurrentMode))
	{
		ToolPaletteBuilders[CurrentMode].ExecuteIfBound(Builder, TabBox);
	}

	Slate_ToolkitDockTab->SetContent(TabBox);
}

void UZenoEditorToolkit::RequestToClose()
{
	if (const auto Tab = this->Slate_ToolkitDockTab; Tab.IsValid())
	{
		Tab->RequestCloseTab();
	}
	UnRegister();
}

void UZenoEditorToolkit::AddMode(const FName InNewMode, const TSharedRef<FUICommandInfo> CommandInfo, const FZenoEditorToolkitBuildToolPalette Builder)
{
	ToolPaletteBuilders.Add(InNewMode, Builder);
	ModeInfos.Add(InNewMode, CommandInfo);
}

void UZenoEditorToolkit::OnChangeMode(const FName ModeName)
{
	CurrentMode = ModeName;
	MakeDockTab(CurrentMode);
}

bool UZenoEditorToolkit::IsModeEnabled(FName ModeName) const
{
	return true;
}

bool UZenoEditorToolkit::IsModeActive(FName ModeName) const
{
	return ModeName == CurrentMode;
}

TSharedRef<SDockTab> UZenoEditorToolkit::GetDockTab(const FSpawnTabArgs& Args)
{
	Slate_ToolkitDockTab = SNew(SDockTab).TabRole(NomadTab);
	MakeDockTab(CurrentMode);
	return Slate_ToolkitDockTab.ToSharedRef();
}
