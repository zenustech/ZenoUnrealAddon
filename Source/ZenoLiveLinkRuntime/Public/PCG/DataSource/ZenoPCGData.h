// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PCG/PCGTypes.h"
#include "UObject/Object.h"
#include <atomic>
#include "ZenoPCGData.generated.h"

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoPCGData : public UObject
{
	GENERATED_BODY()

public:
	UZenoPCGData(const FObjectInitializer& ObjectInitializer);

	virtual EZenoPCGDataType GetDataType() const { return EZenoPCGDataType::None; }

	/** Unique ID of this object. */
	UPROPERTY(Transient)
	uint64 UID = 0;

	/** CRC of this object. */
	mutable FZenoPCGCrc Crc;

private:
	/** Serves unique ID values to instances of this object. */
	static inline std::atomic<uint64> UIDCounter{ 1 };
};
