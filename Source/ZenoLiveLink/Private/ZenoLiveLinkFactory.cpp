// Fill out your copyright notice in the Description page of Project Settings.
#define LOCTEXT_NAMESPACE "ZenoLiveLink"

#include "ZenoLiveLinkFactory.h"

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

	const TSharedPtr<FZenoLiveLinkSource> SourcePtr = MakeShared<FZenoLiveLinkSource>(InSettings);
	OnSourceCreated.ExecuteIfBound(SourcePtr, MoveTemp(ConnectionString));
}

#undef LOCTEXT_NAMESPACE
