#include "UI/Landscape/LandscapeToolZenoBridge.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "EditorDialogLibrary.h"
#include "SlateOptMacros.h"
#include "EditorModeManager.h"
#include "EditorModes.h"
#include "IContentBrowserSingleton.h"
#include "Landscape.h"
#include "LandscapeEdit.h"
#include "LandscapeEditorUtils.h"
#include "LandscapeProxy.h"
#include "Selection.h"
#include "ZenoLiveLink.h"
#include "ZenoLiveLinkSource.h"
#include "Asset/ZenoBridgeAssetFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Classes/ZenoBridgeAsset.h"
#include "Command/FZenoLandscapeCommand.h"
#include "Role/LiveLinkTextureRole.h"
#include "Role/ZenoLiveLinkTypes.h"
#include "UI/Landscape/ZenoLandscapeEditorObject.h"
#include "UI/Landscape/ZenoLandscapeHelper.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SScrollBox.h"

#define LOCTEXT_NAMESPACE "UZenoLandscapeTool"

UZenoLandscapeTool::UZenoLandscapeTool(const FObjectInitializer& ObjectInitializer)
	: UZenoEditorToolkit(ObjectInitializer)
{
	UISetting = NewObject<UZenoLandscapeEditorObject>();
	DisplayName = LOCTEXT("ZenoLandscapeToolName", "Zeno Landscape");
	TabId = TabName;
}

void UZenoLandscapeTool::Init()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    DetailsViewArgs.bAllowSearch = false;
	
	Slate_DetailPanel = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	Slate_DetailPanel->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateUObject(this, &UZenoLandscapeTool::GetIsPropertyVisible));
	check(IsValid(UISetting));
	Slate_DetailPanel->SetObject(UISetting);
	
	AddMode(FName(NAME_ImportHeightfield), FZenoLandscapeCommand::Get().ImportLiveLinkHeightmap.ToSharedRef(), FZenoEditorToolkitBuildToolPalette::CreateUObject(this, &UZenoLandscapeTool::Slate_MarkImportHeightfield));
	AddMode(FName(NAME_ExportWeightmap), FZenoLandscapeCommand::Get().ExportLiveLinkWeightmap.ToSharedRef(), FZenoEditorToolkitBuildToolPalette::CreateUObject(this, &UZenoLandscapeTool::Slate_MarkExportWeightmap));
	AddMode(FName(NAME_VisualLandscapeLayer), FZenoLandscapeCommand::Get().VisualLayer.ToSharedRef(), FZenoEditorToolkitBuildToolPalette::CreateUObject(this, &UZenoLandscapeTool::Slate_VisualLayer));

	// ModeChangedDelegate.AddStatic(&UZenoLandscapeTool::SetSharedCurrentMode);
	ModeChangedDelegate.AddLambda([this] (const FName InName)
	{
		SetSharedCurrentMode(InName);
		if (Slate_DetailPanel.IsValid()) Slate_DetailPanel->ForceRefresh();
	});
}

bool UZenoLandscapeTool::CanBeCreate(const FSpawnTabArgs& Args)
{
	return GLevelEditorModeTools().IsModeActive(FBuiltinEditorModes::EM_Landscape);
}

bool UZenoLandscapeTool::GetIsPropertyVisible(const FPropertyAndParent& PropertyAndParent) const
{
	const FProperty& Property = PropertyAndParent.Property;
	if (Property.HasMetaData("ShowForTools"))
	{
		TArray<FString> ShowForTools;
		Property.GetMetaData("ShowForTools").ParseIntoArray(ShowForTools, TEXT(","), true);
		FString CurrentModeText = CurrentMode.ToString().Replace(TEXT("ZT_Landscape_"), TEXT(""));
		if (ShowForTools.Contains(CurrentModeText))
		{
			return true;
		}
	}

	return false;
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void UZenoLandscapeTool::Slate_MarkImportHeightfield(FToolBarBuilder& ToolBarBuilder,
                                                     const TSharedRef<SVerticalBox> Container)
{
	Slate_DetailPanel->ForceRefresh();
	Container
	->AddSlot()
	.AutoHeight()
	[
		Slate_DetailPanel.ToSharedRef()
	]
	;
	Container
	->AddSlot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(FText::FromString("Import"))
			.OnClicked(FOnClicked::CreateLambda([this]
			{
				ImportHeightMapFromSubject();
				return FReply::Handled();
			}))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(FText::FromString("Save"))
			.OnClicked(FOnClicked::CreateLambda([this]
			{
				SaveHeightMapToAsset();
				return FReply::Handled();
			}))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(FText::FromString("Reimport"))
			.OnClicked(FOnClicked::CreateLambda([this]
			{
				ReimportHeightMapToSelectedLandscape();
				return FReply::Handled();
			}))
		]
	]
	;
}

void UZenoLandscapeTool::Slate_MarkExportWeightmap(FToolBarBuilder& ToolBarBuilder,
	const TSharedRef<SVerticalBox> Container)
{
	Slate_DetailPanel->ForceRefresh();
	Container
	->AddSlot()
	.AutoHeight()
	[
		Slate_DetailPanel.ToSharedRef()
	]
	;
}

void UZenoLandscapeTool::Slate_VisualLayer(FToolBarBuilder& ToolBarBuilder, const TSharedRef<SVerticalBox> Container)
{
	Slate_DetailPanel->ForceRefresh();
	Container
	->AddSlot()
	.AutoHeight()
	[
		Slate_DetailPanel.ToSharedRef()
	]
	;
}

bool UZenoLandscapeTool::CheckSubjectKey() const
{
	if (UISetting->SelectedSubjectKey.SubjectName.IsNone())
	{
		UEditorDialogLibrary::ShowMessage(
			LOCTEXT("ActionWarning", "Warning"),
			LOCTEXT("SubjectKeyIsNone", "You must select a subject to perform this action."),
			EAppMsgType::Ok
		);
		return false;
	}
	return true;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void UZenoLandscapeTool::ImportHeightMapFromSubject()
{
	if (!CheckSubjectKey())
	{
		return;
	}
	if (const TOptional<FLiveLinkSubjectFrameData> FrameData = FZenoCommonDataSource::GetFrameData(UISetting->SelectedSubjectKey); FrameData.IsSet())
	{
		const FLiveLinkHeightFieldStaticData* Data = FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>();

		const int32 SizeX = UISetting->ImportLandscape_ComponentCount.X * UISetting->ImportLandscape_QuadsPerSection + 1;
		const int32 SizeY = UISetting->ImportLandscape_ComponentCount.Y * UISetting->ImportLandscape_QuadsPerSection + 1;
		
		TArray<uint16> HeightData;
		TArray<uint16> ExpandedHeightData;
		HeightData.SetNumUninitialized(Data->Data.Num());
		for (size_t Idx = 0; Idx < Data->Data.Num(); ++Idx)
		{
			HeightData[Idx] = FZenoLandscapeHelper::RemapFloatToUint16(Data->Data[Idx]);
		}
		FZenoLandscapeHelper::ExpandHeightmapData(HeightData, {SizeX, SizeY}, ExpandedHeightData);

		TMap<FGuid, TArray<uint16>> HeightDataPerLayers;
		TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataPerLayers;
		TArray<FLandscapeImportLayerInfo> LandscapeImportLayerInfos;

		HeightDataPerLayers.Add({ FGuid(), ExpandedHeightData });
		MaterialLayerDataPerLayers.Add({FGuid(), LandscapeImportLayerInfos});
		
		FScopedTransaction Transaction(LOCTEXT("Undo", "Creating New Landscape"));

		ALandscape* Landscape = GWorld->SpawnActor<ALandscape>(UISetting->NewLandscape_Location, UISetting->NewLandscape_Rotation);
		Landscape->bCanHaveLayersContent = UISetting->bImportLandscape_CanHaveLayer;
		Landscape->LandscapeMaterial = UISetting->ImportLandscape_MaterialInterface.Get(false);
		Landscape->SetActorRelativeScale3D(UISetting->ImportLandscape_Scale);
		Landscape->StaticLightingLOD = FMath::DivideAndRoundUp(FMath::CeilLogTwo((SizeX * SizeY) / (2048 * 2048) + 1), static_cast<uint32>(2));

		Landscape->Import(FGuid::NewGuid(), 0, 0, SizeX - 1, SizeY - 1, UISetting->ImportLandscape_SectionsPerComponent, UISetting->ImportLandscape_QuadsPerSection, HeightDataPerLayers, *FString::Printf(TEXT("zeno://%s.subject"), *UISetting->SelectedSubjectKey.SubjectName.ToString()), MaterialLayerDataPerLayers, ELandscapeImportAlphamapType::Additive);
		
		ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
		check(LandscapeInfo);

		FActorLabelUtilities::SetActorLabelUnique(Landscape, ALandscape::StaticClass()->GetName());

		LandscapeInfo->UpdateLayerInfoMap(Landscape);
	}
}

void UZenoLandscapeTool::SaveHeightMapToAsset()
{
	if (!CheckSubjectKey())
	{
		return;
	}
	if (const TOptional<FLiveLinkSubjectFrameData> FrameData = FZenoCommonDataSource::GetFrameData(UISetting->SelectedSubjectKey); FrameData.IsSet())
	{
		const FLiveLinkHeightFieldStaticData* Data = FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>();
		
		const FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
		const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		
		UZenoBridgeAssetFactory* ZenoFactory = NewObject<UZenoBridgeAssetFactory>();
		UZenoBridgeAsset* Asset = Cast<UZenoBridgeAsset>(AssetToolsModule.Get().CreateAssetWithDialog(UZenoBridgeAsset::StaticClass(), ZenoFactory));
		if (IsValid(Asset))
		{
			Asset->SetAssetType(EZenoAssetType::HeightField);
			UZenoBridgeAssetData_Heightfield* Data_Heightfield = NewObject<UZenoBridgeAssetData_Heightfield>(Asset);
			Data_Heightfield->Size = sqrt(Data->Size);
			Data_Heightfield->Heights.SetNumUninitialized(Data->Data.Num());
			for (size_t Idx = 0; Idx < Data->Data.Num(); ++Idx)
			{
				 Data_Heightfield->Heights[Idx] = FZenoLandscapeHelper::RemapFloatToUint16(Data->Data[Idx]);;
			}
			Asset->SetAssetData(Data_Heightfield);
		
			FAssetRegistryModule::AssetCreated(Asset);
			if (IsValid(Asset->GetPackage()))
			{
				 auto _ = Asset->GetPackage()->MarkPackageDirty();
			}
			TArray<UObject*> Assets;
			Assets.Push(Asset);
			ContentBrowserModule.Get().SyncBrowserToAssets(Assets);
		}
	}
}

void UZenoLandscapeTool::ReimportHeightMapToSelectedLandscape()
{
	if (!CheckSubjectKey())
	{
		return;
	}
	TArray<ALandscapeProxy*> SelectedLandscapes;
	USelection* SelectedActors = GEditor->GetSelectedActors();
	SelectedActors->GetSelectedObjects<ALandscapeProxy>(SelectedLandscapes);
	if (SelectedLandscapes.Num() == 0)
	{
		UEditorDialogLibrary::ShowMessage(
			LOCTEXT("ActionWarning", "Warning"),
			LOCTEXT("MustSelectALandscape", "You must select a landscape in level editor to perform this action."),
			EAppMsgType::Ok
		);
		return;
	}
	if (const TOptional<FLiveLinkSubjectFrameData> FrameData = FZenoCommonDataSource::GetFrameData(UISetting->SelectedSubjectKey); FrameData.IsSet())
	{
		FScopedTransaction Transaction(LOCTEXT("UndoReimport", "Reimporting Landscape"));
		const FLiveLinkHeightFieldStaticData* Data = FrameData->StaticData.Cast<FLiveLinkHeightFieldStaticData>();
		int32 Size = sqrt(Data->Size);
		TArray<uint16> HeightData;
		HeightData.SetNumUninitialized(Data->Data.Num());
		for (size_t Idx = 0; Idx < Data->Data.Num(); ++Idx)
		{
			HeightData[Idx] = FZenoLandscapeHelper::RemapFloatToUint16(Data->Data[Idx]);
		}
		for (ALandscapeProxy* LandscapeProxy : SelectedLandscapes)
		{
			ULandscapeInfo* LandscapeInfo = LandscapeProxy->GetLandscapeInfo();
			FHeightmapAccessor<false> HeightmapAccessor(LandscapeInfo);
			HeightmapAccessor.SetData(0, 0, Size - 1, Size - 1, HeightData.GetData());
			LandscapeInfo->UpdateLayerInfoMap(LandscapeProxy);
		}
	}
}

FOnClicked UZenoLandscapeTool::CreateOnSubjectListItemClicked(const FLiveLinkSubjectKey& Key)
{
	TWeakObjectPtr<UZenoLandscapeTool> WeakThat { this };
	return FOnClicked::CreateLambda([WeakThat, Key]
	{
		if (auto* That = WeakThat.Get())
		{
			That->UISetting->SelectedSubjectKey = Key;
			if (const auto ComboPtr = That->Slate_SubjectListComboButton)
			{
				ComboPtr->SetIsOpen(false);
			}
			That->ImportHeightMapFromSubject();
		}
		return FReply::Handled();
	});
}

void UZenoLandscapeTool::SetSharedCurrentMode(const FName InName)
{
	LandscapeToolCurrentMode = InName;
}

#undef LOCTEXT_NAMESPACE
