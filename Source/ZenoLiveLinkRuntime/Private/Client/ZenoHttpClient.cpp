// Fill out your copyright notice in the Description page of Project Settings.


#include "Client/ZenoHttpClient.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "ZenoEngine/Public/ThirdParty/msgpack.h"

UZenoHttpClient::UZenoHttpClient()
	: BaseURL("http://localhost:23343")
{
}

void UZenoHttpClient::SetBaseEndpoint(const FString& InBaseEndpoint)
{
	BaseURL = InBaseEndpoint;
}

void UZenoHttpClient::FetchSessionKey(const FString& InToken)
{
	const FRequest Request = CreateNewRequest("/auth", EZenoHttpVerb::Get);
	Request->SetHeader(TEXT("X-Zeno-Token"), InToken);
	Request->OnProcessRequestComplete().BindLambda([this]
	(	
		FHttpRequestPtr Req,
		FHttpResponsePtr Res,
		bool ConnectedSuccessfully
	) mutable
	{
		if (ConnectedSuccessfully && IsValid(this))
		{
			 const TArray<uint8>& Content = Res->GetContent();
			 SessionKey = FString { Content.Num(), reinterpret_cast<const ANSICHAR*>(Content.GetData()) };
		}
	});
	Request->ProcessRequest();
}

bool UZenoHttpClient::HasValidSession() const
{
	return !SessionKey.IsEmpty();
}

TSharedPromise<zeno::remote::Diff> UZenoHttpClient::GetDiffFromRemote(int32 LocalVersion/* = 0 */) const
{
	const FRequest Request = CreateNewRequest("/subject/diff", EZenoHttpVerb::Get, { { "client_version", FString::Printf(TEXT("%d"), LocalVersion) } });
	const TSharedPromise<zeno::remote::Diff> Promise = CreateNewPromise<zeno::remote::Diff>();
	Request->OnProcessRequestComplete().BindLambda(BuildProcessResponse(Promise));
	Request->ProcessRequest();
	return Promise;
}

TSharedPromise<zeno::remote::SubjectContainerList> UZenoHttpClient::GetDataFromRemote(
	const TArray<FString>& InSubjectNames) const
{
	TArray<FZenoLiveLinkKeyValuePair> Params;
	Params.Reserve(InSubjectNames.Num());
	for (const FString& Name : InSubjectNames)
	{
		Params.Add({ "key", Name });
	}
	const FRequest Request = CreateNewRequest("/subject/fetch", EZenoHttpVerb::Get, Params);
	const TSharedPromise<zeno::remote::SubjectContainerList> Promise = CreateNewPromise<zeno::remote::SubjectContainerList>();
	Request->OnProcessRequestComplete().BindLambda(BuildProcessResponse(Promise));
	Request->ProcessRequest();
	return Promise;
}

TSharedPromise<bool> UZenoHttpClient::SetSubjectToRemote(
	zeno::remote::SubjectContainerList& InList) const
{
	const FRequest Request = CreateNewRequest("/subject/push", EZenoHttpVerb::Post);
	const TSharedPromise<bool> Promise = CreateNewPromise<bool>();
	std::vector<uint8> Buffer = msgpack::pack(InList);
	const TArray UBuffer { Buffer.data(), static_cast<TArray<uint8>::SizeType>(Buffer.size()) };
	Request->SetContent(UBuffer);
	Request->OnProcessRequestComplete().BindLambda([Promise] (
		FHttpRequestPtr Req,
		FHttpResponsePtr Res,
		bool ConnectedSuccessfully) mutable
	{
		TOptional Result{ false };
		if (ConnectedSuccessfully)
		{
			Result = Res->GetResponseCode() == 204;
		}
		Promise->SetValue(Result);
	});
	Request->ProcessRequest();
	return Promise;
}

TSharedPromise<bool> UZenoHttpClient::RunGraph(zeno::remote::GraphRunInfo& RunInfo) const
{
	const FRequest Request = CreateNewRequest("/graph/run", EZenoHttpVerb::Post);
	const TSharedPromise<bool> Promise = CreateNewPromise<bool>();
	std::vector<uint8> Buffer = msgpack::pack(RunInfo);
	const TArray UBuffer { Buffer.data(), static_cast<TArray<uint8>::SizeType>(Buffer.size()) };
	Request->SetContent(UBuffer);
	Request->OnProcessRequestComplete().BindLambda([Promise] (
		FHttpRequestPtr Req,
		FHttpResponsePtr Res,
		bool ConnectedSuccessfully) mutable
	{
		TOptional Result{ false };
		if (ConnectedSuccessfully)
		{
			Result = Res->GetResponseCode() == 204;
		}
		Promise->SetValue(Result);
	});
	Request->ProcessRequest();
	return Promise;
}

TSharedPromise<zeno::remote::GraphInfo> UZenoHttpClient::TryParseGraphInfo(const FString& InGraphJson) const
{
	const FRequest Request = CreateNewRequest("/graph/parse", EZenoHttpVerb::Post);
	const TSharedPromise<zeno::remote::GraphInfo> Promise = CreateNewPromise<zeno::remote::GraphInfo>();

	const TArray<uint8> Data { InGraphJson.GetCharArray() };
	Request->SetContent(Data);

	Request->OnProcessRequestComplete().BindLambda(BuildProcessResponse(Promise));

	Request->ProcessRequest();

	return Promise;
}

UZenoHttpClient::FRequest UZenoHttpClient::CreateNewRequest(const FString& InPath, EZenoHttpVerb InVerb/* = EZenoHttpVerb::Get */, const TArray<FZenoLiveLinkKeyValuePair>& InParam/* = {} */) const
{
	FHttpModule& HttpModule = FHttpModule::Get();
	FRequest NewRequest = HttpModule.CreateRequest();
	const UEnum* VerbEnum = FindObject<UEnum>(nullptr, TEXT("/Script/ZenoLiveLinkRuntime.EZenoHttpVerb"), false);
	check(VerbEnum);
	NewRequest->SetVerb(VerbEnum->GetNameStringByValue(static_cast<int64>(InVerb)).ToUpper());
	const FString ParamString = ParamString.JoinBy(InParam, TEXT("&"), FZenoLiveLinkKeyValuePair::ToString);
	NewRequest->SetURL(FString::Printf(TEXT("%s%s%s%s"), *BaseURL, *InPath, InParam.IsEmpty() ? TEXT("") : TEXT("?"), *ParamString));
	NewRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-zeno-msgpack"));
	NewRequest->SetTimeout(5.0f);
	NewRequest->SetHeader(TEXT("X-Zeno-SessionKey"), SessionKey);
	return NewRequest;
}
