﻿#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PCGTypes.generated.h"

// There are some helper classes copy from PCG plugin for backward compatibility.

UENUM(meta = (Bitflags))
enum class EZenoPCGDataType : uint32
{
	None = 0 UMETA(Hidden),
	Point = 1 << 1,
	Volume = 1 << 2,
	Primitive = 1 << 3,
	Landscape = 1 << 4,
	Texture = 1 << 5,
	RenderTarget = 1 << 6,
	Composite = 1 << 7 UMETA(Hidden),

	BaseTexture = Texture | RenderTarget UMETA(Hidden),
	Surface = Landscape | BaseTexture,

	Concrete = Point | Volume | Primitive | Surface,

	Spatial = Composite | Concrete,

	Any = (1 << 30) - 1,
};

/**
 * Crc with valid flag and helper functionality.
 **/
USTRUCT()
struct ZENOLIVELINKRUNTIME_API FZenoPCGCrc
{
	GENERATED_BODY()

	/** Creates an invalid Crc. */
	FZenoPCGCrc() = default;

	/** Initializes a valid Crc set to InValue. */
	explicit FZenoPCGCrc(uint32 InValue)
		: Value(InValue)
		, bValid(true)
	{
	}

	bool IsValid() const { return bValid; }

	uint32 GetValue() const { return Value; }

	/** Combines another Crc into this Crc to chain them. */
	void Combine(const FZenoPCGCrc& InOtherCrc)
	{
#if WITH_EDITOR
		ensure(IsValid() && InOtherCrc.IsValid());
#endif

		Value = HashCombineFast(Value, InOtherCrc.Value);
	}

	/** Combines another Crc value into this Crc to chain them. */
	void Combine(uint32 InOtherCrcValue)
	{
#if WITH_EDITOR
		ensure(IsValid());
#endif

		Value = HashCombineFast(Value, InOtherCrcValue);
	}

	/** Compares Crc. This and other Crc must be valid. */
	bool operator==(const FZenoPCGCrc& InOtherCrc) const
	{
#if WITH_EDITOR
		ensure(IsValid() && InOtherCrc.IsValid());
#endif

		return IsValid() && InOtherCrc.IsValid() && Value == InOtherCrc.Value;
	}

private:
	/** Crc32 value. */
	UPROPERTY(VisibleAnywhere, Category = Crc)
	uint32 Value = 0;

	UPROPERTY(VisibleAnywhere, Category = Crc)
	bool bValid = false;
};

USTRUCT(BlueprintType)
struct ZENOLIVELINKRUNTIME_API FZenoPCGPoint
{
	GENERATED_BODY()
	
	FZenoPCGPoint() = default;
	FZenoPCGPoint(const FTransform& InTransform, float InDensity, int32 InSeed);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	FTransform Transform;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	float Density = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	FVector BoundsMin = -FVector::One();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	FVector BoundsMax = FVector::One();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	FVector4 Color = FVector4::One();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties, meta = (ClampMin = "0", ClampMax = "1"))
	float Steepness = 0.5f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Properties)
	int32 Seed = 0;
};

UENUM()
enum class EZenoMetadataOp : uint8
{
	/** Take the minimum value. */
	Min,
	/** Take the maximum value. */
	Max,
	/** Subtract the values. */
	Sub,
	/** Add the values. */
	Add,
	/** Multiply the values. */
	Mul,
	/** Divide the values. */
	Div,
	/** Pick the source (first) value. */
	SourceValue,
	/** Pick the target (second) value. */
	TargetValue
};

UENUM()
enum class EZenoMetadataFilterMode : uint8
{
	/** The listed attributes will be unchanged by the projection and will not be added from the target data. */
	ExcludeAttributes,
	/** Only the listed attributes will be changed by the projection or added from the target data. */
	IncludeAttributes,
};

USTRUCT(BlueprintType)
struct ZENOLIVELINKRUNTIME_API FZenoPCGProjectionParams
{
	GENERATED_BODY()

	/** Project positions. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apply Data")
	bool bProjectPositions = true;

	/** Project rotations. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apply Data")
	bool bProjectRotations = true;

	/** Project scales. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apply Data")
	bool bProjectScales = false;

	/** Project colors. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apply Data")
	bool bProjectColors = false;

	/** Attributes to either explicitly exclude or include in the projection operation, depending on the Attribute Mode setting. Leave empty to gather all attributes and their values. Format is comma separated list like: Attribute1,Attribute2 .*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apply Data")
	FString AttributeList = TEXT("");

	/** How the attribute list is used. Exclude Attributes will ignore these attributes and their values on the projection target. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apply Data")
	EZenoMetadataFilterMode AttributeMode = EZenoMetadataFilterMode::ExcludeAttributes;

	/** Operation to use to combine attributes that reside on both source and target data. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Apply Data")
	EZenoMetadataOp AttributeMergeOperation = EZenoMetadataOp::TargetValue;
};

USTRUCT(BlueprintType)
struct ZENOLIVELINKRUNTIME_API FZenoPCGContext
{
	GENERATED_BODY()
	
};
