#include "UI/Toolkit/ZenoEditorToolkitBase.h"

#define LOCTEXT_NAMESPACE "UZenoEditorToolkitBase"

UZenoEditorToolkitBase::UZenoEditorToolkitBase()
	: CurrentMode(NAME_None)
	, UICommandList(MakeShared<FUICommandList>())
{
}

FText UZenoEditorToolkitBase::GetDisplayName() const
{
	return LOCTEXT("ZenoEditorToolkit", "Zeno Editor Toolkit");
}

UZenoEditorToolkitBase& UZenoEditorToolkitBase::AddSubMode(const FName& InModeName, const TSharedRef<FUICommandInfo> CommandInfo)
{
	SubModes.Add(InModeName, CommandInfo);
	if (CurrentMode.IsNone())
	{
		CurrentMode = InModeName;
	}
	return *this;
}

UZenoEditorToolkitBase& UZenoEditorToolkitBase::SetPropertyObject(UObject* Object)
{
	check(IsInGameThread());
	PropertyObject = Object;
	return *this;
}

FName UZenoEditorToolkitBase::GetCurrentMode() const
{
	return CurrentMode;
}

bool UZenoEditorToolkitBase::IsInSubMode(const FName InModeName) const
{
	return CurrentMode == InModeName;
}

bool UZenoEditorToolkitBase::CheckPropertyVisibility(const FPropertyAndParent& InPropertyAndParent) const
{
	static CONSTEXPR const TCHAR* ShowForModesName = TEXT("ShowForModes");
	const FProperty& Property = InPropertyAndParent.Property;
	if (Property.HasMetaData(ShowForModesName))
	{
		TArray<FString> ShowForModes;
		Property.GetMetaData(ShowForModesName).ParseIntoArray(ShowForModes, TEXT(","), true);
		if (ShowForModes.Contains(GetCurrentMode().ToString()))
		{
			return true;
		}
	}
	return false;
}

bool UZenoEditorToolkitBase::CanBeCreate(const FSpawnTabArgs& Args) const
{
	PURE_VIRTUAL(UZenoEditorToolkitBase::CanBeCreate);
	return false;
}

FName UZenoEditorToolkitBase::GetUniqueName() const
{
	PURE_VIRTUAL(UZenoEditorToolkitBase::GetUniqueID);
	return NAME_None;
}

TSharedRef<SDockTab> UZenoEditorToolkitBase::GetDockTab(const FSpawnTabArgs& Args)
{
	if (!Slate_ToolkitDockTab.IsValid())
	{
		MakeDockTab();
	}
	return Slate_ToolkitDockTab.ToSharedRef();
}

void UZenoEditorToolkitBase::MakeDockTab()
{
	checkf(IsValid(PropertyObject), TEXT("Must call SetPropertyObject() to provide a valid uobject."));
	
	Slate_ToolkitDockTab = SNew(SDockTab).TabRole(NomadTab);
	const TSharedRef<SVerticalBox> TabBox = SNew(SVerticalBox);
	Slate_ToolkitDockTab->SetContent(TabBox);

	FUniformToolBarBuilder CategoriesBuilder { UICommandList, FMultiBoxCustomization::None };
	CategoriesBuilder.SetStyle(&FAppStyle::Get(), "PaletteToolBar");
	for (const auto& Cate : SubModes )
	{
		UICommandList->MapAction(Cate.Value, FUIAction {
			FExecuteAction::CreateUObject(this, &UZenoEditorToolkitBase::OnChangeMode, Cate.Key),
			FCanExecuteAction::CreateUObject(this, &UZenoEditorToolkitBase::IsSubModeEnable, Cate.Key),
			FIsActionChecked::CreateUObject(this, &UZenoEditorToolkitBase::IsInSubMode, Cate.Key),
		});
#ifdef UE_5_2_OR_LATER
		CategoriesBuilder.AddToolbarStackButton(Cate.Value);
#else
		CategoriesBuilder.AddToolBarButton(Cate.Value);
#endif
	}
	TabBox->AddSlot().AutoHeight() [ CategoriesBuilder.MakeWidget(nullptr, 20) ];
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bAllowSearch = false;

	Slate_DetailPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	Slate_DetailPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateUObject(
		this, &UZenoEditorToolkitBase::CheckPropertyVisibility));
	Slate_DetailPanel->SetObject(PropertyObject);
	TabBox->AddSlot().AutoHeight() [ Slate_DetailPanel.ToSharedRef() ];
}

bool UZenoEditorToolkitBase::IsSubModeEnable(const FName InModeName)
{
	return true;
}

void UZenoEditorToolkitBase::OnChangeMode(const FName ModeName)
{
	CurrentMode = ModeName;
	// Update detail panel with new current mode state
	if (Slate_DetailPanel.IsValid())
	{
		Slate_DetailPanel->ForceRefresh();
	}
	// TODO [darc] : Add mode changed delegate :
}

void UZenoEditorToolkitBase::Register()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		GetUniqueName(),
		FOnSpawnTab::CreateUObject(this, &UZenoEditorToolkitBase::GetDockTab),
		FCanSpawnTab::CreateUObject(this, &UZenoEditorToolkitBase::CanBeCreate)
	)
	.SetDisplayName(GetDisplayName())
	.SetMenuType(ETabSpawnerMenuType::Hidden);
	Init();
}

void UZenoEditorToolkitBase::Unregister()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(GetUniqueName());
}

void UZenoEditorToolkitBase::Init()
{
	PURE_VIRTUAL(UZenoEditorToolkitBase::Init);
}

#undef LOCTEXT_NAMESPACE
