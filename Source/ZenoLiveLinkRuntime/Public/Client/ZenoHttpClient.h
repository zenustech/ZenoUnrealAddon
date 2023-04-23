// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <zeno/unreal/ZenoRemoteTypes.h>

#include "CoreMinimal.h"
#include "ZenoLiveLinkTypes.h"
#include "Interfaces/IHttpRequest.h"
#include "UObject/Object.h"
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
	
	UZenoHttpClient();

	UFUNCTION(BlueprintCallable)
	void SetBaseEndpoint(const FString& InBaseEndpoint);

	TSharedFuture<TOptional<zeno::remote::Diff>> GetDiffFromRemote(int32 LocalVersion = 0);

protected:
	FRequest CreateNewRequest(const FString& InPath, EZenoHttpVerb InVerb = EZenoHttpVerb::Get, const TArray<FZenoLiveLinkKeyValuePair>& InParam = {}) const;

private:
	UPROPERTY(VisibleAnywhere, Category = Zeno)
	FString BaseURL;
};
