#include "UI/Landscape/LandscapeToolZenoBridge.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "ZenoLiveLink.h"
#include "ZenoLiveLinkSource.h"
#include "Asset/ZenoBridgeAssetFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Classes/ZenoBridgeAsset.h"
#include "Role/LiveLinkTextureRole.h"
#include "Role/ZenoLiveLinkTypes.h"
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

void UZenoLandscapeTool::ImportHeightMapFromSubject()
{
	if (!SelectedSubjectKey.IsSet())
	{
		return;
	}
	if (const TOptional<FLiveLinkSubjectFrameData> FrameData = GetFrameData(SelectedSubjectKey.GetValue()); FrameData.IsSet())
	{
		const FLiveLinkHeightFieldStaticData* Data = FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>();

		TArray<uint16> HeightData;
		HeightData.SetNumUninitialized(Data->Data.Num());
		for (size_t Idx = 0; Idx < Data->Data.Num(); ++Idx)
		{
			 const float Height = (Data->Data[Idx] + 255.f) / 512.f * 0xFFFF;
			 HeightData[Idx] = static_cast<uint16>(Height);
		}
		int32 Size = sqrt(Data->Size);

		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		TArray<FLandscapeImportLayerInfo> LandscapeImportLayerInfos;

		HeightDataPerLayers.Add({ FGuid(), HeightData });
		MaterialLayerDataPerLayers.Add({FGuid(), LandscapeImportLayerInfos});
		
		FScopedTransaction Transaction(LOCTEXT("Undo", "Creating New Landscape"));

		ALandscape* Landscape = GWorld->SpawnActor<ALandscape>(FVector::ZeroVector, FRotator::ZeroRotator);
		Landscape->bCanHaveLayersContent = false;
		Landscape->LandscapeMaterial = nullptr;
		Landscape->SetActorRelativeScale3D(FVector{ 100, 100, 32 });
		Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((Size * Size) / (2048 * 2048) + 1), (uint32)2);
		
		Landscape->Import(FGuid::NewGuid(), 0, 0, Size - 1, Size - 1, 1, 7, HeightDataPerLayers, TEXT(""), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive);
		
		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
		check(LandscapeInfo);

		FActorLabelUtilities::SetActorLabelUnique(Landscape, ALandscape::StaticClass()->GetName());

		LandscapeInfo->UpdateLayerInfoMap(Landscape);
		// const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		// const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		//
		// UZenoBridgeAssetFactory* ZenoFactory = NewObject<UZenoBridgeAssetFactory>();
		// UZenoBridgeAsset* Asset = Cast<UZenoBridgeAsset>(AssetToolsModule.Get().CreateAssetWithDialog(UZenoBridgeAsset::StaticClass(), ZenoFactory));
		// if (IsValid(Asset))
		// {
		// 	Asset->SetAssetType(EZenoAssetType::HeightField);
		// 	UZenoBridgeAssetData_Heightfield* Data_Heightfield = NewObject<UZenoBridgeAssetData_Heightfield>(Asset);
		// 	Data_Heightfield->Size = sqrt(Data->Size);
		// 	Data_Heightfield->Heights.SetNumUninitialized(Data->Data.Num());
		// 	for (size_t Idx = 0; Idx < Data->Data.Num(); ++Idx)
		// 	{
		// 		const float Height = (Data->Data[Idx] + 255.f) / 512.f * 0xFFFF;
		// 		Data_Heightfield->Heights[Idx] = static_cast<uint16>(Height);
		// 	}
		// 	Asset->SetAssetData(Data_Heightfield);
		//
		// 	FAssetRegistryModule::AssetCreated(Asset);
		// 	if (IsValid(Asset->GetPackage()))
		// 	{
		// 		Asset->GetPackage()->MarkPackageDirty();
		// 	}
		// 	TArray<UObject*> Assets;
		// 	Assets.Push(Asset);
		// 	ContentBrowserModule.Get().SyncBrowserToAssets(Assets);
		// }
	}
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
			That->ImportHeightMapFromSubject();
		}
		return FReply::Handled();
	});
}

#undef LOCTEXT_NAMESPACE
