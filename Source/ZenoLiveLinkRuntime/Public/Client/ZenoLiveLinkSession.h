// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoLiveLinkTypes.h"
#include "UObject/Object.h"
#include <atomic>

#include "ZenoHttpClient.h"
THIRD_PARTY_INCLUDES_START
#include "zeno/unreal/ZenoRemoteTypes.h"
THIRD_PARTY_INCLUDES_END

#include "ZenoLiveLinkSession.generated.h"

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoLiveLinkSession : public UObject
{
	GENERATED_BODY()

	UZenoLiveLinkSession();

public:
	/**
	 * @brief Call to initialize the connection
	 * Should only been called once.
	 * @param ConnectionSetting 
	 */
	void Init(const FZenoLiveLinkSetting& ConnectionSetting);

	UZenoHttpClient* GetClient() const;

	bool IsInitialized() const;

	bool HasSubject(const FString& InName) const;

protected:
	std::atomic<bool> bInitialized;
	
	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = Zeno)
	FGuid Guid;
	
	UPROPERTY(EditAnywhere, Category = Zeno)
	FZenoLiveLinkSetting Settings;

	UPROPERTY(VisibleAnywhere, AdvancedDisplay, Category = Zeno)
	UZenoHttpClient* HttpClient;

	TMap<FString, zeno::remote::SubjectContainer> OwnedSubjects;

public:
	static UZenoLiveLinkSession* CreateSession(const FZenoLiveLinkSetting& ConnectionSetting, FGuid Guid);
	
};
