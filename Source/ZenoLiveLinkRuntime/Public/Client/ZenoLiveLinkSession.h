// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoLiveLinkTypes.h"
#include "UObject/Object.h"
#include <atomic>
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
	
protected:
	std::atomic<bool> bInitialized;
	
	UPROPERTY(VisibleAnywhere, AdvancedDisplay)
	FGuid Guid;
	
	UPROPERTY(EditAnywhere, Category = Zeno)
	FZenoLiveLinkSetting Settings;

public:
	static UZenoLiveLinkSession* CreateSession(const FZenoLiveLinkSetting& ConnectionSetting, FGuid Guid);
	
};
