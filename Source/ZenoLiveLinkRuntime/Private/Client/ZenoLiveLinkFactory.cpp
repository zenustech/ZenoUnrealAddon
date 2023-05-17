// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/ZenoLiveLinkFactory.h"

#include "Client/ZenoLiveLinkClientSubsystem.h"
#include "Client/ZenoLiveLinkSource.h"
#include "Client/ZenoLiveLinkTypes.h"
#include "Client/Slate/SZenoLiveLinkCreationPanel.h"
#include "GenericPlatform/GenericPlatformOutputDevices.h"

#define LOCTEXT_NAMESPACE "UZenoLiveLinkFactory"

FText UZenoLiveLinkFactory::GetSourceDisplayName() const
{
	return LOCTEXT("ZenoSourceDisplayName", "Zeno");
}

FText UZenoLiveLinkFactory::GetSourceTooltip() const
{
	return LOCTEXT("ZenoSourceTooltip", "Start a new zeno remote session.");
}

#ifdef WITH_EDITORONLY_DATA
TSharedPtr<SWidget> UZenoLiveLinkFactory::BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const
{
	return SNew(SZenoLiveLinkCreationPanel)
			.OnConnectionSettingsAccepted_UObject(this, &UZenoLiveLinkFactory::CreateSourceFromSettings, OnLiveLinkSourceCreated);
}
#endif // WITH_EDITORONLY_DATA

TSharedPtr<ILiveLinkSource> UZenoLiveLinkFactory::CreateSource(const FString& ConnectionString) const
{
	FZenoLiveLinkSetting Setting;
	FZenoLiveLinkSetting::StaticStruct()->ImportText(*ConnectionString, &Setting, const_cast<UZenoLiveLinkFactory*>(this), CPF_Transient, FGenericPlatformOutputDevices::GetError(), FZenoLiveLinkSetting::StaticStruct()->GetStructCPPName(), true);
	UZenoLiveLinkClientSubsystem* Subsystem = GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>();
	const FGuid NewId = Subsystem->NewSession(Setting);
	TSharedRef<FZenoLiveLinkSource> Source = MakeShared<FZenoLiveLinkSource>(NewId);
	return Source;
}

ULiveLinkSourceFactory::EMenuType UZenoLiveLinkFactory::GetMenuType() const
{
	return EMenuType::SubPanel;
}

void UZenoLiveLinkFactory::CreateSourceFromSettings(FZenoLiveLinkSetting InSettings,
                                                    FOnLiveLinkSourceCreated OnSourceCreated) const
{
	FString ConnectionString;
	FZenoLiveLinkSetting::StaticStruct()->ExportText(ConnectionString, &InSettings, nullptr, nullptr, PPF_None, nullptr);

	const TSharedRef<ILiveLinkSource> NewSource = CreateSource(ConnectionString).ToSharedRef();
	OnSourceCreated.ExecuteIfBound(NewSource, MoveTemp(ConnectionString));
}

#undef LOCTEXT_NAMESPACE
