// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/Slate/SZenoLiveLinkCreationPanel.h"

#if WITH_EDITOR
#include "IStructureDetailsView.h"
#include "PropertyEditorModule.h"
#endif // WITH_EDITOR
#include "SlateOptMacros.h"
#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Client/ZenoLiveLinkTypes.h"

#define LOCTEXT_NAMESPACE "SZenoLiveLinkCreationPanel"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SZenoLiveLinkCreationPanel::Construct(const FArguments& InArgs)
{
#if WITH_EDITOR
	OnConnectionSettingsAccepted = InArgs._OnConnectionSettingsAccepted;

	FPropertyEditorModule& PropertyEditor = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	const UZenoLiveLinkClientSubsystem* ZenoClientSubsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	check(IsValid(ZenoClientSubsystem));

	FStructureDetailsViewArgs StructureDetailsViewArgs;
	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bShowScrollBar = false;
	DetailsViewArgs.bAllowSearch = false;

	StructOnScope = MakeShared<FStructOnScope>(FZenoLiveLinkSetting::StaticStruct());
	CastChecked<UScriptStruct>(StructOnScope->GetStruct())->CopyScriptStruct(StructOnScope->GetStructMemory(), &ZenoClientSubsystem->GetZenoLiveLinkSetting());
	StructureDetailsView = PropertyEditor.CreateStructureDetailView(DetailsViewArgs, StructureDetailsViewArgs, StructOnScope);
	
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			StructureDetailsView->GetWidget().ToSharedRef()
		]
		+ SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoHeight()
		[
			SNew(SButton)
			.OnClicked(this, &SZenoLiveLinkCreationPanel::OnSettingAccepted)
			.Text(LOCTEXT("Accept", "Accept"))
		]
	];
#endif // WITH_EDITOR
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SZenoLiveLinkCreationPanel::OnSettingAccepted() const
{
#if WITH_EDITOR
	UZenoLiveLinkClientSubsystem* ZenoClientSubsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	check(IsValid(ZenoClientSubsystem));
	CastChecked<UScriptStruct>(StructOnScope->GetStruct())->CopyScriptStruct(&ZenoClientSubsystem->GetZenoLiveLinkSetting(), StructOnScope->GetStructMemory());
	OnConnectionSettingsAccepted.ExecuteIfBound(ZenoClientSubsystem->GetZenoLiveLinkSetting());
#endif // WITH_EDITOR
	
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
