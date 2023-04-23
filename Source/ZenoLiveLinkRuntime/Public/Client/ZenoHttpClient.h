// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <system_error>
#include <zeno/unreal/ZenoRemoteTypes.h>

#include "CoreMinimal.h"
#include "ZenoLiveLinkTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "UObject/Object.h"
#include "ZenoEngine/Public/ThirdParty/msgpack.h"
#include "ZenoHttpClient.generated.h"

/**
 * Provide interface to interact with zeno
 */
UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoHttpClient : public UObject
{
	GENERATED_BODY()

public:
	using FRequest = TSharedRef<IHttpRequest, ESPMode::ThreadSafe>;
	template <typename T>
	using TAsyncResult = TSharedFuture<TOptional<T>>;
	template <typename T>
	using TSharedPromise = TSharedRef<TPromise<TOptional<T>>>;
	
	UZenoHttpClient();

	UFUNCTION(BlueprintCallable)
	void SetBaseEndpoint(const FString& InBaseEndpoint);

	TAsyncResult<zeno::remote::Diff> GetDiffFromRemote(int32 LocalVersion = 0) const;

	TAsyncResult<zeno::remote::SubjectContainerList> GetDataFromRemote(const TArray<FString>& InSubjectNames);

protected:
	FRequest CreateNewRequest(const FString& InPath, EZenoHttpVerb InVerb = EZenoHttpVerb::Get, const TArray<FZenoLiveLinkKeyValuePair>& InParam = {}) const;

	template <typename T>
	static TSharedPromise<T> CreateNewPromise();

	template <typename T>
	static auto BuildProcessResponse(TSharedPromise<T> Context);

private:
	UPROPERTY(VisibleAnywhere, Category = Zeno)
	FString BaseURL;
};

template <typename T>
UZenoHttpClient::TSharedPromise<T> UZenoHttpClient::CreateNewPromise()
{
	TSharedPromise<T> Promise = MakeShared<TPromise<TOptional<T>>>();
	return Promise;
}

template <typename T>
auto UZenoHttpClient::BuildProcessResponse(TSharedPromise<T> Context)
{
	return [Context] (
		FHttpRequestPtr Req,
		FHttpResponsePtr Res,
		bool ConnectedSuccessfully) mutable
	{
		TOptional<T> Result;
		if (ConnectedSuccessfully)
		{
			const TArray<uint8>& Content = Res->GetContent();
			std::error_code Err;
			Result = msgpack::unpack<T>(Content.GetData(), Content.Num(), Err);
			if (Err)
			{
				Result.Reset();
			}
		}
		Context->SetValue(Result);
	};
}
