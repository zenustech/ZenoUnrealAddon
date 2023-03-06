// Fill out your copyright notice in the Description page of Project Settings.


#include "Asset/ZenoBridgeAssetFactory.h"

#include "Classes/ZenoBridgeAsset.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

#define LOCTEXT_NAMESPACE "ZenoBridgeAssetFactory"

UZenoBridgeAssetFactory::UZenoBridgeAssetFactory()
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UZenoBridgeAsset::StaticClass();
}

bool UZenoBridgeAssetFactory::ShouldShowInNewMenu() const
{
	return true;
}

UObject* UZenoBridgeAssetFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn, FName CallingContext)
{
	check(InClass->IsChildOf(UZenoBridgeAsset::StaticClass()));
	return NewObject<UZenoBridgeAsset>(InParent, InClass, InName, Flags | RF_Transactional);
}

void FZenoBridgeAssetActions::OpenAssetEditor(const TArray<UObject*>& InObjects,
                                              TSharedPtr<IToolkitHost> EditWithinLevelEditor)
{
	EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		const auto ZenoBridgeAsset = Cast<UZenoBridgeAsset>(*ObjIt);

		if (ZenoBridgeAsset != nullptr)
		{
			TSharedRef<FZenoBridgeAssetToolkits> EditorToolkit = MakeShareable(new FZenoBridgeAssetToolkits());
			EditorToolkit->InitZenoBridgeEditor(Mode, EditWithinLevelEditor, ZenoBridgeAsset);
		}
	}
}

FLinearColor FZenoBridgeAssetToolkits::GetWorldCentricTabColorScale() const
{
	return FLinearColor::Gray;
}

FName FZenoBridgeAssetToolkits::GetToolkitFName() const
{
	return FName("ZenoBridgeAssetToolkit");
}

FText FZenoBridgeAssetToolkits::GetBaseToolkitName() const
{
	return LOCTEXT("AssetToolkitsBaseName", "Zeno bridge asset toolkit");
}

FString FZenoBridgeAssetToolkits::GetWorldCentricTabPrefix() const
{
	return "Zeno";
}

void FZenoBridgeAssetToolkits::InitZenoBridgeEditor(const EToolkitMode::Type InToolkitMode,
	const TSharedPtr<IToolkitHost>& InitToolkitHost, UZenoBridgeAsset* InAsset)
{
	WeakAsset = InAsset;
	TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("NullLayout")->AddArea(FTabManager::NewPrimaryArea());
	InitAssetEditor(InToolkitMode, InitToolkitHost, FName("ZenoBridgeAssetEditorApp"), Layout, true, true, InAsset);
	DocumentTracker = MakeShared<FDocumentTracker>();
	DocumentTracker->Initialize(SharedThis(this));
	DocumentTracker->SetTabManager(TabManager.ToSharedRef());
}

FText FZenoBridgeAssetActions::GetName() const
{
	return LOCTEXT("AssetActionName", "ZenoBridge");
}

UClass* FZenoBridgeAssetActions::GetSupportedClass() const
{
	return UZenoBridgeAsset::StaticClass();
}

FColor FZenoBridgeAssetActions::GetTypeColor() const
{
	return FColor::Orange;
}

uint32 FZenoBridgeAssetActions::GetCategories()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	return AssetTools.RegisterAdvancedAssetCategory("Zeno", LOCTEXT("AssetCategoryName", "Zeno"));
}

#undef LOCTEXT_NAMESPACE
