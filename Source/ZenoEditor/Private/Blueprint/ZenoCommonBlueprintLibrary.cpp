// Fill out your copyright notice in the Description page of Project Settings.


#include "Blueprint/ZenoCommonBlueprintLibrary.h"

#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "IContentBrowserSingleton.h"
#include "IDesktopPlatform.h"
#include "IStructureDetailsView.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Interfaces/IMainFrameModule.h"
#include "Widgets/Notifications/SNotificationList.h"

#define LOCTEXT_NAMESPACE "UZenoCommonBlueprintLibrary"

bool UZenoCommonBlueprintLibrary::OpenSystemFilePicker(TArray<FString>& OutFiles, const FString& Title /** = "Open File" */, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, const int32 Flags)
{
	if (IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get())
	{
		 return DesktopPlatform->OpenFileDialog(nullptr, Title, DefaultPath, DefaultFile, FileTypes, Flags, OutFiles);
	}
	return false;
}

FString UZenoCommonBlueprintLibrary::OpenContentPicker(const FString& AssetName, const FString& PackagePath)
{
	FSaveAssetDialogConfig SaveAssetDialogConfig;
	SaveAssetDialogConfig.DialogTitleOverride = LOCTEXT("SelectPackage", "Select Package");
	SaveAssetDialogConfig.DefaultPath = PackagePath;
	SaveAssetDialogConfig.DefaultAssetName = AssetName;
	SaveAssetDialogConfig.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;

	const FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(SaveAssetDialogConfig);
	return SaveObjectPath;
}

bool UZenoCommonBlueprintLibrary::OpenSettingsModal(UObject* InObject, const FText& InTitle /** = LOCTEXT("ModalTitle", "") */)
{
	ensure(IsValid(InObject));

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView({});
	DetailsView->SetObject(InObject);
	TSharedPtr<SWindow> ModalWindow;
	TSharedPtr<SVerticalBox> ContextBox;

	bool bFlag = false;
	
	const auto CloseThisWindow = [&ModalWindow] ()
	{
		if (ModalWindow.IsValid())
		{
			ModalWindow->RequestDestroyWindow();
		}
	};

	
	SAssignNew(ContextBox, SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		 DetailsView
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		 SNew(SHorizontalBox)
		 + SHorizontalBox::Slot()
		 .AutoWidth()
		 [
			   SNew(SButton)
			   .Text(LOCTEXT("Apply", "Apply"))
			   .OnClicked_Lambda([&bFlag, CloseThisWindow] () mutable
			   {
					bFlag = true;
					CloseThisWindow();
					return FReply::Handled();
			   })
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			  SNew(SButton)
			  .Text(LOCTEXT("Cancel", "Cancel"))
			  .OnClicked_Lambda([CloseThisWindow] { CloseThisWindow(); return FReply::Handled(); })
		]
	];

	// Force update desired size in current tick
	ContextBox->SlatePrepass(ContextBox->GetTickSpaceGeometry().Scale);

	SAssignNew(ModalWindow, SWindow)
		.Title(InTitle)
		.Content()
		[
			ContextBox.ToSharedRef()
		]
		.ClientSize(FVector2D(1200, 600))
		.SupportsMaximize(true)
		.IsInitiallyMinimized(false)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SizingRule(ESizingRule::Autosized)
	;

	const IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
	const TSharedPtr<const SWidget> ParentWidget = MainFrame.GetParentWindow();
	
	FSlateApplication::Get().AddModalWindow(ModalWindow.ToSharedRef(), ParentWidget, false);
	return bFlag;
}

bool UZenoCommonBlueprintLibrary::OpenSettingsModal(TSharedRef<FStructOnScope> InObject, const FText& InTitle)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	const TSharedRef<IStructureDetailsView> DetailsView = PropertyEditorModule.CreateStructureDetailView({}, {}, InObject);
	TSharedPtr<SWindow> ModalWindow;
	TSharedPtr<SVerticalBox> ContextBox;

	bool bFlag = false;
	
	const auto CloseThisWindow = [&ModalWindow] ()
	{
		if (ModalWindow.IsValid())
		{
			ModalWindow->RequestDestroyWindow();
		}
	};

	
	SAssignNew(ContextBox, SVerticalBox)
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		 DetailsView->GetWidget().ToSharedRef()
	]
	+ SVerticalBox::Slot()
	.AutoHeight()
	[
		 SNew(SHorizontalBox)
		 + SHorizontalBox::Slot()
		 .AutoWidth()
		 [
			   SNew(SButton)
			   .Text(LOCTEXT("Apply", "Apply"))
			   .OnClicked_Lambda([&bFlag, CloseThisWindow] () mutable
			   {
					bFlag = true;
					CloseThisWindow();
					return FReply::Handled();
			   })
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			  SNew(SButton)
			  .Text(LOCTEXT("Cancel", "Cancel"))
			  .OnClicked_Lambda([CloseThisWindow] { CloseThisWindow(); return FReply::Handled(); })
		]
	];

	// Force update desired size in current tick
	ContextBox->SlatePrepass(ContextBox->GetTickSpaceGeometry().Scale);

	SAssignNew(ModalWindow, SWindow)
		.Title(InTitle)
		.Content()
		[
			ContextBox.ToSharedRef()
		]
		.ClientSize(FVector2D(1200, 600))
		.SupportsMaximize(true)
		.IsInitiallyMinimized(false)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SizingRule(ESizingRule::Autosized)
	;

	const IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
	const TSharedPtr<const SWidget> ParentWidget = MainFrame.GetParentWindow();
	
	FSlateApplication::Get().AddModalWindow(ModalWindow.ToSharedRef(), ParentWidget, false);
	return bFlag;
}

void UZenoCommonBlueprintLibrary::ShowNotification(const FText& InText, const float InDuration)
{
	FNotificationInfo NotificationInfo(InText);
	NotificationInfo.ExpireDuration = InDuration;

	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}

#undef LOCTEXT_NAMESPACE
