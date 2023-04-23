#pragma once

#include "ZenoLiveLinkTypes.generated.h"

UENUM(BlueprintType)
enum class EZenoHttpProtocolType : uint8
{
	Http = 0,
	Https,
};

UENUM(BlueprintType)
enum class EZenoHttpVerb : uint8
{
	Get = 0,
	Post,
	Put,
	Patch,
	Delete,
	Copy,
	Head,
	Options,
	Link,
	Unlink,
	Purge,
	Lock,
	Unlock,
	Propfind,
	View,
};

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

	/** While deploy zeno server to public, it might behind reserve proxy */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	EZenoHttpProtocolType Protocol = EZenoHttpProtocolType::Http;

	/** Auth token */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString Token = TEXT("ZENO_DEFAULT_TOKEN");
};

USTRUCT(BlueprintType)
struct ZENOLIVELINKRUNTIME_API FZenoLiveLinkKeyValuePair
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = Zeno)
	FString Key;
	UPROPERTY(EditAnywhere, Category = Zeno)
	FString Value;

	static FString ToString(const FZenoLiveLinkKeyValuePair& InSelf)
	{
		return FString::Printf(TEXT("%s=%s"), *InSelf.Key, *InSelf.Value);
	}
};

template <typename T>
struct TWrappedPrimitiveType
{
	T Data;

	TWrappedPrimitiveType(T InData) : Data(InData) {}

	T operator*() { return Data; }
	T operator=(T NewData) { return Data = NewData; }
};
