#pragma once

#include "ZenoLiveLinkTypes.generated.h"

USTRUCT(BlueprintType)
struct ZENOLIVELINKRUNTIME_API FZenoLiveLinkSetting
{
	GENERATED_BODY()

	/** IP address of the zeno source */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString IPAddress = TEXT("127.0.0.1");

	/** TCP port number */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	uint16 HTTPPortNumber = 23343;

	/** Auth token */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString Token = TEXT("ZENO_DEFAULT_TOKEN");
};
