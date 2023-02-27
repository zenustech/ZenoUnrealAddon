// Fill out your copyright notice in the Description page of Project Settings.
#define LOCTEXT_NAMESPACE "ZenoLiveLink"

#include "ZenoLiveLinkFactory.h"

#include "Helper.h"
#include "ZenoBridge.h"
#include "ZenoLiveLinkSource.h"
#include "UI/SZenoLiveLinkSourceFactory.h"

FText UZenoLiveLinkFactory::GetSourceDisplayName() const
{
	return LOCTEXT("ZenoSourceDisplayName", "Zeno");
}

FText UZenoLiveLinkFactory::GetSourceTooltip() const
{
	return LOCTEXT("ZenoSourceTooltip", "Allow recieve procedural from zeno core");
}

TSharedPtr<SWidget> UZenoLiveLinkFactory::CreateSourceCreationPanel()
{
	return nullptr;
}

TSharedPtr<ILiveLinkSource> UZenoLiveLinkFactory::OnSourceCreationPanelClosed(bool bMakeSource)
{
	return nullptr;
}

TSharedPtr<ILiveLinkSource> UZenoLiveLinkFactory::CreateSource(const FString& ConnectionString) const
{
	// FZenoLiveLinkSetting::StaticStruct()->ImportText(*ConnectionString, );
	return nullptr;
}

TSharedPtr<SWidget> UZenoLiveLinkFactory::BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const
{
	return SNew(SZenoLiveLinkSourceFactory)
		.OnConnectionSettingsAccepted(FOnZenoLiveLinkConnectionSettingsAccepted::CreateUObject(this, &UZenoLiveLinkFactory::CreateSourceFromSettings, OnLiveLinkSourceCreated));
}

void UZenoLiveLinkFactory::CreateSourceFromSettings(
	FZenoLiveLinkSetting InSettings,
	const FOnLiveLinkSourceCreated OnSourceCreated) const
{
	FString ConnectionString;
	FZenoLiveLinkSetting::StaticStruct()->ExportText(ConnectionString, &InSettings, nullptr, nullptr, PPF_None, nullptr);

	FUnrealSocketHelper::AuthToken = TCHAR_TO_UTF8(*InSettings.Token);
	FZenoBridgeModule& ZenoBridge = FModuleManager::Get().GetModuleChecked<FZenoBridgeModule>("ZenoBridge");
	if (ZenoBridge.StartClient(FString::Printf(TEXT("%ls:%d"), *InSettings.IPAddress, InSettings.TCPPortNumber)))
	{
		const TSharedPtr<FZenoLiveLinkSource> SourcePtr = MakeShared<FZenoLiveLinkSource>(InSettings);
		OnSourceCreated.ExecuteIfBound(SourcePtr, MoveTemp(ConnectionString));
	}
	else
	{
		// TODO: darc show error dialog
	}
}

#undef LOCTEXT_NAMESPACE
