// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ZenoLiveLinkSetting.generated.h"

USTRUCT()
struct ZENOLIVELINK_API FZenoLiveLinkSetting
{
	GENERATED_BODY()

public:
	/** IP address of the zeno source */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString IPAddress = TEXT("127.0.0.1");

	/** TCP port number */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	uint16 TCPPortNumber = 23343;

	/** Auth token */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString Token = TEXT("123456");
};
