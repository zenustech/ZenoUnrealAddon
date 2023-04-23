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

UZenoHttpClient::TAsyncResult<zeno::remote::Diff> UZenoHttpClient::GetDiffFromRemote(int32 LocalVersion/* = 0 */) const
{
	const FRequest Request = CreateNewRequest("/subject/diff", EZenoHttpVerb::Get, { { "client_version", FString::Printf(TEXT("%d"), LocalVersion) } });
	const TSharedPromise<zeno::remote::Diff> Promise = CreateNewPromise<zeno::remote::Diff>();
	TAsyncResult<zeno::remote::Diff> OutFuture = Promise->GetFuture().Share();
	Request->OnProcessRequestComplete().BindLambda(BuildProcessResponse(Promise));
	Request->ProcessRequest();
	return OutFuture;
}

UZenoHttpClient::TAsyncResult<zeno::remote::SubjectContainerList> UZenoHttpClient::GetDataFromRemote(
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
	TAsyncResult<zeno::remote::SubjectContainerList> OutFuture = Promise->GetFuture().Share();
	Request->OnProcessRequestComplete().BindLambda(BuildProcessResponse(Promise));
	Request->ProcessRequest();
	return OutFuture;
}

UZenoHttpClient::TAsyncResult<bool> UZenoHttpClient::SetSubjectToRemote(
	zeno::remote::SubjectContainerList& InList) const
{
	const FRequest Request = CreateNewRequest("/subject/push", EZenoHttpVerb::Post);
	const TSharedPromise<bool> Promise = CreateNewPromise<bool>();
	TAsyncResult<bool> OutFuture = Promise->GetFuture().Share();
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
	return OutFuture;
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
	return NewRequest;
}
