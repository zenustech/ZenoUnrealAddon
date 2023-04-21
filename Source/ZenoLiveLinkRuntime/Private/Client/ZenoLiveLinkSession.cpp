#include "Client/ZenoLiveLinkSession.h"

UZenoLiveLinkSession::UZenoLiveLinkSession()
{
}

void UZenoLiveLinkSession::Init(const FZenoLiveLinkSetting& ConnectionSetting)
{
	check(!bInitialized.load());
	bInitialized = true;
	
	Settings = ConnectionSetting;

	HttpClient = httplib::Client(std::string { TCHAR_TO_ANSI(*FString::Printf(TEXT("http://%s:%d"), *Settings.IPAddress, Settings.HTTPPortNumber)) });
	// TODO [darc] : Request for session key :
}

UZenoLiveLinkSession* UZenoLiveLinkSession::CreateSession(const FZenoLiveLinkSetting& ConnectionSetting, const FGuid Guid)
{
	UZenoLiveLinkSession* NewSession = NewObject<UZenoLiveLinkSession>();
	NewSession->Init(ConnectionSetting);
	NewSession->Guid = Guid;
	return NewSession;
}
