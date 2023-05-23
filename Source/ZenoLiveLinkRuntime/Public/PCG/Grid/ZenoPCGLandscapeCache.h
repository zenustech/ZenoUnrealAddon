// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZenoPCGLandscapeCache.generated.h"

class ALandscapeProxy;
class UZenoMetadata;
class ULandscapeComponent;
class ULandscapeInfo;
struct FZenoPCGPoint;

namespace ZenoPCGLandscapeCache
{
	struct FSafeIndices
	{
		int32 X0Y0 = 0;
		int32 X1Y0 = 0;
		int32 X0Y1 = 0;
		int32 X1Y1 = 0;

		float XFraction = 0;
		float YFraction = 0;
	};

	// this will ensure all indicies are valid in a Stride*Stride sized array
	FSafeIndices CalcSafeIndices(FVector2D LocalPosition, int32 Stride);
};

struct FZenoPCGLandscapeCacheEntry
{
public:
	void GetInterpolatedPointHeightOnly(const FVector2D& LocalPoint, FZenoPCGPoint& OutPoint) const;

	/** Returns true if the data was loaded from the bulk data - non editor only */
	bool TouchAndLoad(int32 InTouch) const;
	/** Returns the size of the serialized data in bytes */
	int32 GetMemorySize() const;

private:
	void GetInterpolatedPointInternal(const ZenoPCGLandscapeCache::FSafeIndices& Indices, FZenoPCGPoint& OutPoint, bool bHeightOnly = false) const;

protected:
#if WITH_EDITOR
	static FZenoPCGLandscapeCacheEntry* CreateCacheEntry(ULandscapeInfo* LandscapeInfo, ULandscapeComponent* InComponent);
#endif // WITH_EDITOR
	
	// Serialize called from the landscape cache
	void Serialize(FArchive& Ar, UObject* Owner, int32 BulkIndex);

	// Internal usage methods
	void SerializeToBulkData();
	void SerializeFromBulkData() const;

	// Serialized data
	TWeakObjectPtr<const ULandscapeComponent> Component = nullptr;
	TArray<FName> LayerDataNames;
	FVector PointHalfSize = FVector::One();
	int32 Stride = 0;

	// Data built in editor or loaded from the bulk data
	mutable FByteBulkData BulkData;

	// Data stored in the BulkData
	mutable TArray<FVector> PositionsAndNormals;
	mutable TArray<TArray<uint8>> LayerData;

	// Transient data
	mutable FCriticalSection DataLock;
	mutable int32 Touch = 0;
	mutable bool bDataLoaded = false;

	friend class UZenoPCGLandscapeCache;
};

UCLASS()
class ZENOLIVELINKRUNTIME_API UZenoPCGLandscapeCache : public UObject
{
	GENERATED_BODY()
	
public:
	void ClearCache();
	void AdvanceCache();
	
	//~Begin UObject interface
	virtual void BeginDestroy() override;
	virtual void Serialize(FArchive& Archive) override;
	//~End UObject interface

#if WITH_EDITOR
	/** Gets (and creates if needed) the cache entry - available only in Editor */
	const FZenoPCGLandscapeCacheEntry* GetCacheEntry(ULandscapeComponent* LandscapeComponent, const FIntPoint& ComponentCoordinate);
#endif // WITH_EDITOR
	
	/** Gets landscape cache entry, works both in editor (but does not create) but works in game mode too. */
	const FZenoPCGLandscapeCacheEntry* GetCacheEntry(const FGuid& LandscapeGuid, const FIntPoint& ComponentCoordinate);

private:
	TMap<TPair<FGuid, FIntPoint>, FZenoPCGLandscapeCacheEntry*> CachedData;

	std::atomic<int32> CacheMemorySize = 0;
	std::atomic<int32> CacheTouch = 0;

	bool bInitialized = false;

#if WITH_EDITOR
	TSet<TWeakObjectPtr<ALandscapeProxy>> Landscapes;
	FRWLock CacheLock;
#endif
};
