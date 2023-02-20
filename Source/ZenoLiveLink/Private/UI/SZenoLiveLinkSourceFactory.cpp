// Fill out your copyright notice in the Description page of Project Settings.
#define LOCTEXT_NAMESPACE "ZenoLiveLinkSourceFactory"

#include "SZenoLiveLinkSourceFactory.h"

#include "PropertyEditorModule.h"
#include "SlateOptMacros.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SZenoLiveLinkSourceFactory::Construct(const FArguments& InArgs)
{
#if WITH_EDITOR
	OnConnectionSettingsAccepted = InArgs._OnConnectionSettingsAccepted;

	// create a detail view widget to edit settings
	FPropertyEditorModule& PropertyEditor = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	FStructureDetailsViewArgs StructureDetailsViewArgs;
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bShowScrollBar = false;
	
	StructOnScope = MakeShared<FStructOnScope>(FZenoLiveLinkSetting::StaticStruct());
	CastChecked<UScriptStruct>(StructOnScope->GetStruct())->CopyScriptStruct(StructOnScope->GetStructMemory(), &ConnectionSettings);
	StructureDetailsView = PropertyEditor.CreateStructureDetailView(DetailsViewArgs, StructureDetailsViewArgs, StructOnScope);

	// add view widget to sub panel
	ChildSlot
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			StructureDetailsView->GetWidget().ToSharedRef()
		]
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoHeight()
		[
			SNew(SButton)
			.OnClicked(this, &SZenoLiveLinkSourceFactory::OnSettingsAccepted)
			.Text(LOCTEXT("AddSourceAccept", "Accept"))
		]
	];

#endif // WITH_EDITOR
}

FReply SZenoLiveLinkSourceFactory::OnSettingsAccepted()
{
#if WITH_EDITOR
	CastChecked<UScriptStruct>(StructOnScope->GetStruct())->CopyScriptStruct(&ConnectionSettings, StructOnScope->GetStructMemory());
	OnConnectionSettingsAccepted.ExecuteIfBound(ConnectionSettings);
#endif // WITH_EDITOR

	return FReply::Handled();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
