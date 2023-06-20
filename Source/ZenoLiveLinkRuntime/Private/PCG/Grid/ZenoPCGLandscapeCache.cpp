// Fill out your copyright notice in the Description page of Project Settings.


#include "PCG/Grid/ZenoPCGLandscapeCache.h"

#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"
#include "LandscapeLayerInfoObject.h"
#include "LandscapeProxy.h"
#include "PCG/PCGTypes.h"
#include "PCG/ZenoLandscapeHelper.h"
#include "Serialization/BufferWriter.h"

ZenoPCGLandscapeCache::FSafeIndices ZenoPCGLandscapeCache::CalcSafeIndices(FVector2D LocalPosition, int32 Stride)
{
	check(Stride != 0);

	const FVector2D ClampedLocalPoint = LocalPosition.ClampAxes(0.0, Stride-1);

	FSafeIndices Result;
	const int32 CellX0 = FMath::FloorToInt(ClampedLocalPoint.X);
	const int32 CellY0 = FMath::FloorToInt(ClampedLocalPoint.Y);
	const int32 CellX1 = FMath::Min(CellX0+1, Stride-1);
	const int32 CellY1 = FMath::Min(CellY0+1, Stride-1);

	Result.X0Y0 = CellX0 + CellY0 * Stride;
	Result.X1Y0 = CellX1 + CellY0 * Stride;
	Result.X0Y1 = CellX0 + CellY1 * Stride;
	Result.X1Y1 = CellX1 + CellY1 * Stride;

	Result.XFraction = FMath::Fractional(ClampedLocalPoint.X);
	Result.YFraction = FMath::Fractional(ClampedLocalPoint.Y);

	return Result;
}

void FZenoPCGLandscapeCacheEntry::GetInterpolatedPointHeightOnly(const FVector2D& LocalPoint,
                                                                 FZenoPCGPoint& OutPoint) const
{
	check(bDataLoaded);
	const ZenoPCGLandscapeCache::FSafeIndices Indices = ZenoPCGLandscapeCache::CalcSafeIndices(LocalPoint, Stride);
	GetInterpolatedPointInternal(Indices, OutPoint, true);
}

bool FZenoPCGLandscapeCacheEntry::TouchAndLoad(int32 InTouch) const
{
	Touch = InTouch;

#if WITH_EDITOR
	check(bDataLoaded);
	return false;
#else
	if (!bDataLoaded)
	{
		FScopeLock ScopeDataLock(&DataLock);
		if (!bDataLoaded)
		{
			SerializeFromBulkData();
			return true;
		}
	}

	return false;
#endif // WITH_EDITOR
}

int32 FZenoPCGLandscapeCacheEntry::GetMemorySize() const
{
	int32 MemSize = PositionsAndNormals.GetAllocatedSize();
	
	for (const TArray<uint8>& Layer : LayerData)
	{
		MemSize += Layer.GetAllocatedSize();
	}
	
	return MemSize;
}

void FZenoPCGLandscapeCacheEntry::GetInterpolatedPointInternal(const ZenoPCGLandscapeCache::FSafeIndices& Indices,
                                                               FZenoPCGPoint& OutPoint, bool bHeightOnly/* = false */) const
{
	check(bDataLoaded);
	check(2 * Indices.X1Y1 < PositionsAndNormals.Num());

	const FVector& PositionX0Y0 = PositionsAndNormals[2 * Indices.X0Y0];
	const FVector& PositionX1Y0 = PositionsAndNormals[2 * Indices.X1Y0];
	const FVector& PositionX0Y1 = PositionsAndNormals[2 * Indices.X0Y1];
	const FVector& PositionX1Y1 = PositionsAndNormals[2 * Indices.X1Y1];

	const FVector LerpPositionY0 = FMath::Lerp(PositionX0Y0, PositionX1Y0, Indices.XFraction);
	const FVector LerpPositionY1 = FMath::Lerp(PositionX0Y1, PositionX1Y1, Indices.XFraction);
	const FVector Position = FMath::Lerp(LerpPositionY0, LerpPositionY1, Indices.YFraction);

	const int32 Seed = Zeno::Helper::ComputeSeed(Position.X, Position.Y, Position.Z);
	const float Density = 1;

	if (bHeightOnly)
	{
		new(&OutPoint) FZenoPCGPoint(FTransform(Position), Density, Seed);
	}
	else
	{
		const FVector& NormalX0Y0 = PositionsAndNormals[2 * Indices.X0Y0 + 1];
		const FVector& NormalX1Y0 = PositionsAndNormals[2 * Indices.X1Y0 + 1];
		const FVector& NormalX0Y1 = PositionsAndNormals[2 * Indices.X0Y1 + 1];
		const FVector& NormalX1Y1 = PositionsAndNormals[2 * Indices.X1Y1 + 1];

		const FVector LerpNormalY0 = FMath::Lerp(NormalX0Y0.GetSafeNormal(), NormalX1Y0.GetSafeNormal(), Indices.XFraction).GetSafeNormal();
		const FVector LerpNormalY1 = FMath::Lerp(NormalX0Y1.GetSafeNormal(), NormalX1Y1.GetSafeNormal(), Indices.XFraction).GetSafeNormal();
		const FVector Normal = FMath::Lerp(LerpNormalY0, LerpNormalY1, Indices.YFraction);

		const FVector TangentX = FVector(Normal.Z, 0.f, -Normal.X);
		const FVector TangentY = Normal ^ TangentX;

		new(&OutPoint) FZenoPCGPoint(FTransform(TangentX.GetSafeNormal(), TangentY.GetSafeNormal(), Normal.GetSafeNormal(), Position), Density, Seed);
	}

	OutPoint.BoundsMin = -PointHalfSize;
	OutPoint.BoundsMax = PointHalfSize;
}

#if WITH_EDITOR
FZenoPCGLandscapeCacheEntry* FZenoPCGLandscapeCacheEntry::CreateCacheEntry(ULandscapeInfo* LandscapeInfo,
	ULandscapeComponent* InComponent)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(FZenoPCGLandscapeCacheEntry::CreateCacheEntry);

	FLandscapeComponentDataInterface DataInterface { InComponent, 0, false };

	// check component has valid texture
	if (!DataInterface.GetRawHeightData())
	{
		return nullptr;
	}
	
	const int32 Stride = InComponent->ComponentSizeQuads + 1;
	if (Stride <= 0)
	{
		return nullptr;
	}

	FZenoPCGLandscapeCacheEntry* NewEntry = new FZenoPCGLandscapeCacheEntry();

	NewEntry->Component = InComponent;
	NewEntry->PointHalfSize = InComponent->GetComponentTransform().GetScale3D() * 0.5;
	NewEntry->Stride = Stride;

	{
		FVector WorldPos;
		FVector WorldTangentX;
		FVector WorldTangentY;
		FVector WorldTangentZ;

		const int32 NumVertices = FMath::Square(Stride);

		NewEntry->PositionsAndNormals.Reserve(2 * NumVertices);

		for (int32 Index = 0; Index < NumVertices; ++Index)
		{
			DataInterface.GetWorldPositionTangents(Index, WorldPos, WorldTangentX, WorldTangentY, WorldTangentZ);
			NewEntry->PositionsAndNormals.Add(WorldPos);
			NewEntry->PositionsAndNormals.Add(WorldTangentZ);
		}
	}

	{
		TArray<uint8> LayerCache;
		for (const FLandscapeInfoLayerSettings& Layer : LandscapeInfo->Layers)
		{
			ULandscapeLayerInfoObject* LayerInfo = Layer.LayerInfoObj;
			if (IsValid(LayerInfo))
			{
				if (DataInterface.GetWeightmapTextureData(LayerInfo, LayerCache, false))
				{
					NewEntry->LayerDataNames.Add(Layer.LayerName);
					NewEntry->LayerData.Emplace(MoveTemp(LayerCache));
				}
				LayerCache.Reset();
			}
		}
	}

	NewEntry->bDataLoaded = true;

	return NewEntry;
}
#endif // WITH_EDITOR

void FZenoPCGLandscapeCacheEntry::Serialize(FArchive& Ar, UObject* Owner, int32 BulkIndex)
{
	if (bDataLoaded && Ar.IsSaving() && Ar.IsCooking())
	{
		SerializeToBulkData();
	}

	Ar << Component;
	Ar << PointHalfSize;
	Ar << Stride;
	Ar << LayerDataNames;

	// We force it not inline that means bulk data won't automatically be loaded when we deserialize
	// later but only when we explicitly take action to load it
	BulkData.SetBulkDataFlags(BULKDATA_Force_NOT_InlinePayload);
	BulkData.Serialize(Ar, Owner, BulkIndex);
}

void FZenoPCGLandscapeCacheEntry::SerializeToBulkData()
{
	// Move data from local arrays to the bulk data
	BulkData.Lock(LOCK_READ_WRITE);

	int32 NumBytes = 0;
	// Number of entries in the array + size of the array
	NumBytes += sizeof(int32) + PositionsAndNormals.Num() * PositionsAndNormals.GetTypeSize();

	// Number of layers
	NumBytes += sizeof(int32); 
	for (TArray<uint8>& CurrentLayerData : LayerData)
	{
		// Number of entries in the layer data array + size of the array
		NumBytes += sizeof(int32) + CurrentLayerData.Num() * CurrentLayerData.GetTypeSize();
	}

	uint8* Dest = (uint8*)BulkData.Realloc(NumBytes);
	FBufferWriter Ar(Dest, NumBytes);
	Ar.SetIsPersistent(true);

	Ar << PositionsAndNormals;
	
	int32 LayerDataCount = LayerData.Num();
	Ar << LayerDataCount;

	for (TArray<uint8>& CurrentLayerData : LayerData)
	{
		Ar << CurrentLayerData;
	}

	BulkData.Unlock();
}

void FZenoPCGLandscapeCacheEntry::SerializeFromBulkData() const
{
	check(!bDataLoaded);

	// Note: this call is not threadsafe by itself, it is meant to be called from a locked region
	uint8* Data = nullptr;
	BulkData.GetCopy(reinterpret_cast<void**>(&Data));
	const int32 DataSize = BulkData.GetBulkDataSize();

	FBufferReader Ar(Data, DataSize, /*bInFreeOnClose=*/true, /*bIsPersistent=*/true);

	Ar << PositionsAndNormals;

	int32 LayerCount = 0;
	Ar << LayerCount;

	LayerData.SetNum(LayerCount);

	for (int32 LayerIndex = 0; LayerIndex < LayerCount; ++LayerIndex)
	{
		Ar << LayerData[LayerIndex];
	}

	bDataLoaded = true;
}

void UZenoPCGLandscapeCache::ClearCache()
{
	for (TPair<TPair<FGuid, FIntPoint>, FZenoPCGLandscapeCacheEntry*>& CachedItem : CachedData)
	{
		delete CachedItem.Value;
		CachedItem.Value = nullptr;
	}

	CachedData.Reset();
}

void UZenoPCGLandscapeCache::AdvanceCache()
{
}

void UZenoPCGLandscapeCache::BeginDestroy()
{
	ClearCache();
#if WITH_EDITOR
	// TeardownLandscapeCallbacks();
#endif
	
	Super::BeginDestroy();
}

void UZenoPCGLandscapeCache::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);

	if (Archive.IsSaving() && !Archive.IsCooking())
	{
		ClearCache();
	}

	int32 NumEntries = Archive.IsLoading() ? 0 : CachedData.Num();
	Archive << NumEntries;

	if (Archive.IsLoading())
	{
		CachedData.Reserve(NumEntries);

		for (int32 EntryIndex = 0; EntryIndex < NumEntries; ++EntryIndex)
		{
			TPair<FGuid, FIntPoint> Key;
			Archive << Key;

			FZenoPCGLandscapeCacheEntry* Entry = new FZenoPCGLandscapeCacheEntry();
			Entry->Serialize(Archive, this, EntryIndex);

			CachedData.Add(Key, Entry);
		}
	}
	else
	{
		int32 EntryIndex = 0;
		for (auto& CachedEntry : CachedData)
		{
			Archive << CachedEntry.Key;
			CachedEntry.Value->Serialize(Archive, this, EntryIndex++);
		}
	}

}

#if WITH_EDITOR
const FZenoPCGLandscapeCacheEntry* UZenoPCGLandscapeCache::GetCacheEntry(ULandscapeComponent* LandscapeComponent,
	const FIntPoint& ComponentCoordinate)
{
	const FGuid LandscapeGuid = (LandscapeComponent && LandscapeComponent->GetLandscapeProxy() ? LandscapeComponent->GetLandscapeProxy()->GetLandscapeGuid() : FGuid());
	const FZenoPCGLandscapeCacheEntry* CacheEntry = GetCacheEntry(LandscapeGuid, ComponentCoordinate);

	if (!CacheEntry && LandscapeComponent && LandscapeComponent->GetLandscapeInfo())
	{
		FWriteScopeLock ScopeLock(CacheLock);
		TPair<FGuid, FIntPoint> ComponentKey { LandscapeGuid, ComponentCoordinate };
		if (FZenoPCGLandscapeCacheEntry** FoundEntry = CachedData.Find(ComponentKey))
		{
			CacheEntry = *FoundEntry;
		}
		else
		{
			check(LandscapeComponent->SectionBaseX / LandscapeComponent->ComponentSizeQuads == ComponentKey.Value.X && LandscapeComponent->SectionBaseY / LandscapeComponent->ComponentSizeQuads == ComponentKey.Value.Y);
			if (FZenoPCGLandscapeCacheEntry* NewEntry = FZenoPCGLandscapeCacheEntry::CreateCacheEntry(LandscapeComponent->GetLandscapeInfo(), LandscapeComponent))
			{
				CacheEntry = NewEntry;
				CachedData.Add(ComponentKey, NewEntry);
			}
		}

		if (CacheEntry)
		{
			if (CacheEntry->TouchAndLoad(CacheTouch++))
			{
				CacheMemorySize += CacheEntry->GetMemorySize();
			}
		}
	}

	return CacheEntry;
}
#endif // WITH_EDITOR

const FZenoPCGLandscapeCacheEntry* UZenoPCGLandscapeCache::GetCacheEntry(const FGuid& LandscapeGuid,
	const FIntPoint& ComponentCoordinate)
{
	const FZenoPCGLandscapeCacheEntry* CacheEntry = nullptr;

	{
		const TPair<FGuid, FIntPoint> ComponentKey {LandscapeGuid, ComponentCoordinate};
#if WITH_EDITOR
		FReadScopeLock ScopeLock(CacheLock);
#endif // WITH_EDITOR
		if (FZenoPCGLandscapeCacheEntry** FoundEntry = CachedData.Find(ComponentKey))
		{
			CacheEntry = *FoundEntry;
		}
	}

	if (CacheEntry)
	{
		if (CacheEntry->TouchAndLoad(CacheTouch++))
		{
			CacheMemorySize += CacheEntry->GetMemorySize();
		}
	}

	return CacheEntry;
}
