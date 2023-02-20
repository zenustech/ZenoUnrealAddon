// Fill out your copyright notice in the Description page of Project Settings.

#define LOCTEXT_NAMESPACE "ZenoLiveLinkSource"

#include "ZenoLiveLinkSource.h"

FZenoLiveLinkSource::FZenoLiveLinkSource(const FZenoLiveLinkSetting InConnectionSettings)
{
	SourceStatus = LOCTEXT("SourceStatus_NoData", "No data");
	SourceType = LOCTEXT("SourceType_Zeno", "Zeno");
	SourceMachineName = FText::Format(LOCTEXT("ZenoSourceMachineName", "{0}:{1}"), FText::FromString(InConnectionSettings.IPAddress), FText::AsNumber(InConnectionSettings.UDPPortNumber, &FNumberFormattingOptions::DefaultNoGrouping()));
}

FZenoLiveLinkSource::~FZenoLiveLinkSource()
{
}

void FZenoLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, const FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}

bool FZenoLiveLinkSource::IsSourceStillValid() const
{
	return false;
}

bool FZenoLiveLinkSource::RequestSourceShutdown()
{
	return true;
}

void FZenoLiveLinkSource::Update()
{
}

#undef LOCTEXT_NAMESPACE
