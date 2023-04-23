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

TSharedFuture<TOptional<zeno::remote::Diff>> UZenoHttpClient::GetDiffFromRemote(int32 LocalVersion/* = 0 */)
{
	const FRequest Request = CreateNewRequest("/subject/diff", EZenoHttpVerb::Get, { { "client_version", "client_version" } });
	TSharedRef<TPromise<TOptional<zeno::remote::Diff>>> Promise = MakeShared<TPromise<TOptional<zeno::remote::Diff>>>();
	TSharedFuture<TOptional<zeno::remote::Diff>> OutFuture = Promise->GetFuture().Share();
	Request->OnProcessRequestComplete().BindLambda([Promise] (
		FHttpRequestPtr Req,
		FHttpResponsePtr Res,
		bool ConnectedSuccessfully) mutable 
	{
		TOptional<zeno::remote::Diff> Result;
		if (ConnectedSuccessfully)
		{
			const TArray<uint8>& Content = Res->GetContent();
			std::error_code Err;
			Result = msgpack::unpack<zeno::remote::Diff>(Content.GetData(), Content.Num(), Err);
			if (Err)
			{
				Result.Reset();
			}
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
