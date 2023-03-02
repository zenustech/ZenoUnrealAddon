#include "UI/Landscape/LandscapeToolZenoBridge.h"

#include "ZenoLiveLink.h"
#include "ZenoLiveLinkSource.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "UZenoLandscapeTool"

UZenoLandscapeTool::UZenoLandscapeTool()
{
}

void UZenoLandscapeTool::Register()
{
	UZenoLandscapeTool* That = NewObject<UZenoLandscapeTool>();
	FZenoLiveLinkModule::GetInstance()->LandscapeTool = TStrongObjectPtr<UZenoLandscapeTool>{ That };
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabName,
		FOnSpawnTab::CreateUObject(That, &UZenoLandscapeTool::MakeDockTab),
		FCanSpawnTab::CreateUObject(That, &UZenoLandscapeTool::ShouldCreateDockTab)
	)
	.SetDisplayName(LOCTEXT("ZenoLandscapeTool", "Zeno Landscape Tool"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void UZenoLandscapeTool::UnRegister()
{
	if (const auto Tab = FZenoLiveLinkModule::GetInstance()->LandscapeTool->Slate_LandscapeToolDockTab; Tab.IsValid())
	{
		Tab.Pin()->RequestCloseTab();
	}
	FZenoLiveLinkModule::GetInstance()->LandscapeTool.Reset();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
}

void UZenoLandscapeTool::InitWidget()
{
	FGlobalTabmanager::Get()->TryInvokeTab(TabName);
}

TSharedRef<SDockTab> UZenoLandscapeTool::MakeDockTab(const FSpawnTabArgs& Args)
{
	TArray<FLiveLinkSubjectKey> SubjectKeys = GetAllSubjectKeys();

	Slate_SubjectListView = SNew(SScrollBox).Orientation(EOrientation::Orient_Vertical);
	for (const auto Key : SubjectKeys)
	{
		Slate_SubjectListView
		->AddSlot()
		.AutoSize()
		[
			SNew(SButton)
			.Text(FText::FromString(Key.SubjectName.ToString()))
			.OnClicked(CreateOnSubjectListItemClicked(Key))
		];
	}

	Slate_SubjectListComboButton = SNew(SComboButton)
		.ButtonContent()
		[
		    SNew(STextBlock)
		    .Text_UObject(this, &UZenoLandscapeTool::GetSubjectComboButtonText)
		]
		.MenuContent()
		[
		    SNew(SVerticalBox)
		    + SVerticalBox::Slot()
		    .MaxHeight(150)
		    [
				Slate_SubjectListView.ToSharedRef()
			]
		];
	
	TSharedRef<SDockTab> DockTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// List height maps
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LiveLinkSubjects", "Target Subject"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				Slate_SubjectListComboButton.ToSharedRef()
			]
		]
		.OnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda( [] (TSharedRef<SDockTab>)
		{
		}))
	;

	Slate_LandscapeToolDockTab = DockTab;

	return DockTab;
}

bool UZenoLandscapeTool::ShouldCreateDockTab(const FSpawnTabArgs& Args)
{
	return true;
}

FText UZenoLandscapeTool::GetSubjectComboButtonText() const
{
	if (SelectedSubjectKey.IsSet())
	{
		return FText::FromString(SelectedSubjectKey->SubjectName.ToString());
	}
	return LOCTEXT("SelectASubject", "Select a subject...");
}

FOnClicked UZenoLandscapeTool::CreateOnSubjectListItemClicked(const FLiveLinkSubjectKey& Key)
{
	TWeakObjectPtr<UZenoLandscapeTool> WeakThis { this };
	return FOnClicked::CreateLambda([WeakThis, Key]
	{
		if (auto* That = WeakThis.Get())
		{
			That->SelectedSubjectKey = Key;
			if (const auto ComboPtr = That->Slate_SubjectListComboButton)
			{
				ComboPtr->SetIsOpen(false);
			}
		}
		return FReply::Handled();
	});
}

#undef LOCTEXT_NAMESPACE
