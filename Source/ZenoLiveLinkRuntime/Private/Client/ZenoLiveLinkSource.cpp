#include "Client/ZenoLiveLinkSource.h"

#include "Client/ZenoLiveLinkClientSubsystem.h"

#define LOCTEXT_NAMESPACE "FZenoLiveLinkSource"

FZenoLiveLinkSource::FZenoLiveLinkSource()
	: Client(nullptr)
{
	SourceStatus = LOCTEXT("SourceStatus_NoData", "No data");
	SourceType = LOCTEXT("SourceType_Zeno", "Zeno");
	SourceMachineName = FText::Format(LOCTEXT("ZenoSourceMachineName", "{0}:{1}"), FText::FromString(GetConnectionSetting().IPAddress), FText::AsNumber(GetConnectionSetting().HTTPPortNumber, &FNumberFormattingOptions::DefaultNoGrouping()));
}

void FZenoLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
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
	// TODO [darc] : Shutdown client :
	return true;
}

void FZenoLiveLinkSource::Update()
{
	// TODO [darc] : Do tick :
}

FZenoLiveLinkSetting& FZenoLiveLinkSource::GetMutableConnectionSetting()
{
	return GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>()->ConnectionSetting;
}

const FZenoLiveLinkSetting& FZenoLiveLinkSource::GetConnectionSetting()
{
	return GEngine->GetEngineSubsystem<UZenoLiveLinkClientSubsystem>()->ConnectionSetting;
}

#undef LOCTEXT_NAMESPACE
