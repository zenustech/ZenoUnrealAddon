// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <zeno/unreal/ZenoRemoteTypes.h>

#include "CoreMinimal.h"
#include "ZenoEngineTypes.generated.h"

class UZenoInputParameter;

// Forward declaration
namespace zeno::remote
{
	struct ParamDescriptor;
}

/**
 * Copy of zeno::remote::EParamType
 */
UENUM()
enum class EZenoParamType : int8
{
	Invalid = -1,
	Float = 0 UMETA(Default),
	Integer,
	Num UMETA(Hidden),
};

/**
 * Copy of zeno::remote::ESubjectType
 */
UENUM()
enum class EZenoSubjectType : int16
{
    Invalid = -1,
    Mesh = 0 UMETA(Default),
    HeightField,
    Num UMETA(Hidden),
};

/**
 * Copy of zeno::remote::ParamDescriptor
 */
USTRUCT(BlueprintType)
struct ZENOENGINE_API FZenoInputParameterDescriptor
{
	GENERATED_BODY()

	/** Parameter name. */
	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Param Name")
	FString Name;

	/** Parameter type. */
	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Param Type")
	EZenoParamType Type = EZenoParamType::Invalid;

	/** Convert from zeno::remote::ParamDescriptor to FZenoInputParameterDescriptor. */
	static FZenoInputParameterDescriptor FromZenoType(const zeno::remote::ParamDescriptor& Descriptor);

	/** Create a new UZenoInputParameter from this descriptor. */
	UZenoInputParameter* CreateInputParameter(UObject* Object) const;
};

/**
 * Copy of zeno::remote::ParamDescriptor
 */
USTRUCT(BlueprintType)
struct ZENOENGINE_API FZenoOutputParameterDescriptor
{
	GENERATED_BODY()

	/** Parameter name. */
	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Param Name")
	FString Name;

	/** Parameter type. */
	UPROPERTY(VisibleAnywhere, Category = Zeno, DisplayName = "Param Type")
	EZenoSubjectType Type = EZenoSubjectType::Invalid;

	/** Convert from zeno::remote::ParamDescriptor to FZenoOutputParameterDescriptor. */
	FORCEINLINE static FZenoOutputParameterDescriptor FromZenoType(const zeno::remote::ParamDescriptor& Descriptor)
	{
		return {
			Descriptor.Name.c_str(),
			static_cast<EZenoSubjectType>(Descriptor.Type),
		};
	}
};
