#include "Client/ZenoLiveLinkSession.h"

UZenoLiveLinkSession::UZenoLiveLinkSession()
{
}

void UZenoLiveLinkSession::Init(const FZenoLiveLinkSetting& ConnectionSetting)
{
	check(!bInitialized.load());
	bInitialized = true;
	
	Settings = ConnectionSetting;
	
	HttpClient = NewObject<UZenoHttpClient>();
	HttpClient->SetBaseEndpoint(FString::Printf(TEXT("http%s://%s:%d"), Settings.Protocol == EZenoHttpProtocolType::Http ? TEXT("") : TEXT("s"), *Settings.IPAddress, Settings.HTTPPortNumber));

	// TODO [darc] : Request for session key :
}

UZenoLiveLinkSession* UZenoLiveLinkSession::CreateSession(const FZenoLiveLinkSetting& ConnectionSetting, const FGuid Guid)
{
	UZenoLiveLinkSession* NewSession = NewObject<UZenoLiveLinkSession>();
	NewSession->Init(ConnectionSetting);
	NewSession->Guid = Guid;
	return NewSession;
}
