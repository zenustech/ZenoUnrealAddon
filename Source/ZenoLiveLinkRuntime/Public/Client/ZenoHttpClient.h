#pragma once

#include <system_error>

#include "zeno/unreal/ZenoRemoteTypes.h"

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

	/**
	 * Set base endpoint of zeno server.
	 * @param InBaseEndpoint Base endpoint of zeno server.
	 */
	UFUNCTION(BlueprintCallable)
	void SetBaseEndpoint(const FString& InBaseEndpoint);

	/**
	 * Fetch session key from zeno server.
	 * @param InToken Token to fetch session key.
	 */
	UFUNCTION(BlueprintCallable)
	void FetchSessionKey(const FString& InToken);

	/**
	 * Check if session key is valid.
	 * @return True if session key is valid.
	 */
	UE_NODISCARD bool HasValidSession() const;

	/**
	 * Get diff from remote.
	 * @param LocalVersion Local version to compare with remote.
	 * @return Diff from remote.
	 */
	UE_NODISCARD TSharedPromise<zeno::remote::Diff> GetDiffFromRemote(int32 LocalVersion = 0) const;

	/**
	 * Get data from remote.
	 * @param InSubjectNames Names of subjects to fetch.
	 * @return Data from remote.
	 */
	UE_NODISCARD TSharedPromise<zeno::remote::SubjectContainerList> GetDataFromRemote(const TArray<FString>& InSubjectNames) const;

	/**
	 * Set subject to remote.
	 * @param InList List of subjects to set.
	 * @return True if success.
	 */
	TSharedPromise<bool> SetSubjectToRemote(zeno::remote::SubjectContainerList& InList) const;

	TSharedPromise<bool> RunGraph(zeno::remote::GraphRunInfo& RunInfo) const;

	/**
	 * Get graph info from remote.
	 * @return Graph info from remote.
	 */
	UE_NODISCARD TSharedPromise<zeno::remote::GraphInfo> TryParseGraphInfo(const FString& InGraphJson) const;

	/**
	 * @brief Utility function to create new promise.
	 * @tparam T Return type of promise.
	 * @return A new promise.
	 */
	template <typename T>
	static TSharedPromise<T> CreateNewPromise();
	
protected:
	/**
	 * @brief Create new request and set some default configuration.
	 * @param InPath Path of request.
	 * @param InVerb Verb of request.
	 * @param InParam Parameters of request.
	 * @return A new request.
	 */
	FRequest CreateNewRequest(const FString& InPath, EZenoHttpVerb InVerb = EZenoHttpVerb::Get, const TArray<FZenoLiveLinkKeyValuePair>& InParam = {}) const;

	/**
	 * @brief Utility function to build process response lambda.
	 * @tparam T Return type of promise.
	 * @param Context Promise to set value.
	 * @return A lambda to process response.
	 */
	template <typename T>
	static auto BuildProcessResponse(TSharedPromise<T> Context);

private:
	/** Base URL of zeno server. */
	UPROPERTY(VisibleAnywhere, Category = Zeno)
	FString BaseURL;

	/** Session key of zeno server. */
	UPROPERTY(VisibleAnywhere, Category = Zeno, AdvancedDisplay)
	FString SessionKey;
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

template <typename T>
using TAsyncResult = UZenoHttpClient::TAsyncResult<T>;

template <typename T>
using TSharedPromise = UZenoHttpClient::TSharedPromise<T>;

template <typename T>
using TResultFuture = TFuture<TOptional<T>>;
