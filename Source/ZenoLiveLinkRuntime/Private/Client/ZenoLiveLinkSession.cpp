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
	HttpClient->FetchSessionKey(ConnectionSetting.Token);
}

UZenoHttpClient* UZenoLiveLinkSession::GetClient() const
{
	return HttpClient;
}

bool UZenoLiveLinkSession::IsInitialized() const
{
	return bInitialized.load();
}

bool UZenoLiveLinkSession::HasSubject(const FString& InName) const
{
	return OwnedSubjects.Contains(InName);
}

UZenoLiveLinkSession* UZenoLiveLinkSession::CreateSession(const FZenoLiveLinkSetting& ConnectionSetting, const FGuid Guid)
{
	UZenoLiveLinkSession* NewSession = NewObject<UZenoLiveLinkSession>();
	NewSession->Init(ConnectionSetting);
	NewSession->Guid = Guid;
	return NewSession;
}
