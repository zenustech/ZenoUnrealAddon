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
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this]
	{
		const auto Future = HttpClient->GetDiffFromRemote();
		Future.Wait();
		const auto Res = Future.Get();
		int32 Output = 0;
		if (Res.IsSet())
		{
			Output = Res->data.size();
		}
		UE_LOG(LogTemp, Warning, TEXT("abab: %d"), Output);
	});

	// TODO [darc] : Request for session key :
}

UZenoLiveLinkSession* UZenoLiveLinkSession::CreateSession(const FZenoLiveLinkSetting& ConnectionSetting, const FGuid Guid)
{
	UZenoLiveLinkSession* NewSession = NewObject<UZenoLiveLinkSession>();
	NewSession->Init(ConnectionSetting);
	NewSession->Guid = Guid;
	return NewSession;
}
