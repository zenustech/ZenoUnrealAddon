// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZenoEngineTypes.generated.h"

// Forward declaration
namespace zeno::remote
{
	struct ParamDescriptor;
}

/**
 * Copy of zeno::remote::EParamType
 */
UENUM(BlueprintType)
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
UENUM(BlueprintType)
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
struct FZenoInputParameterDescriptor
{
	GENERATED_BODY()

	/** Parameter name. */
	UPROPERTY(Category = Zeno, DisplayName = "Param Name")
	FString Name;

	/** Parameter type. */
	UPROPERTY(Category = Zeno, DisplayName = "Param Type")
	EZenoParamType Type;

	/** Convert from zeno::remote::ParamDescriptor to FZenoInputParameterDescriptor. */
	UFUNCTION(BlueprintCallable, Category = Zeno)
	static FZenoInputParameterDescriptor FromZenoType(const zeno::remote::ParamDescriptor& Descriptor);
};

/**
 * Copy of zeno::remote::ParamDescriptor
 */
USTRUCT(BlueprintType)
struct FZenoOutputParameterDescriptor
{
	GENERATED_BODY()

	/** Parameter name. */
	UPROPERTY(Category = Zeno, DisplayName = "Param Name")
	FString Name;

	/** Parameter type. */
	UPROPERTY(Category = Zeno, DisplayName = "Param Type")
	EZenoSubjectType Type;

	/** Convert from zeno::remote::ParamDescriptor to FZenoOutputParameterDescriptor. */
	UFUNCTION(BlueprintCallable, Category = Zeno)
	static FZenoOutputParameterDescriptor FromZenoType(const zeno::remote::ParamDescriptor& Descriptor);
};
